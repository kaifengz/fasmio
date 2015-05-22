
#include "./epoll.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "./fiber-env.h"
#include "./log.h"

namespace fasmio { namespace fiber_env {

const unsigned long kEpollThreadStackSize = 256*1024;

Epoller::Epoller() :
    epoll_fd_(-1),
    buzz_rd_(-1),
    buzz_wr_(-1),
    quit_flag_(false),
    env_(nullptr),
    to_wait_fibers_lock_(),
    to_wait_fibers_(),
    wfibers_by_id_(),
    wfibers_by_time_(),
    epoll_thread_(EpollThreadProc, static_cast<void*>(this), kEpollThreadStackSize)
{
}

Epoller::~Epoller()
{
    Stop();
}

bool Epoller::Start(FiberEnv *env)
{
    if (epoll_fd_ >= 0)
        return false;
    if (env == nullptr)
        return false;
    env_ = env;

    epoll_fd_ = epoll_create(5);
    if (epoll_fd_ < 0)
        return false;

    if (!SetupBuzz())
    {
        close(epoll_fd_);
        epoll_fd_ = -1;
        return false;
    }

    quit_flag_ = false;
    epoll_thread_.Start();
    return true;
}

void Epoller::Stop()
{
    if (epoll_fd_ >= 0)
    {
        quit_flag_ = true;
        RingBuzz();
        epoll_thread_.Join();

        DestroyBuzz();
        close(epoll_fd_);
        epoll_fd_ = -1;
        env_ = nullptr;
    }
}

int Epoller::Wait(int fd, Condition cond, Condition *cond_ret, const char* state)
{
    return Wait(fd, cond, cond_ret, nullptr, state);
}

int Epoller::TimedWait(int fd, Condition cond, Condition *cond_ret, const runtime_env::ABSTime &time, const char* state)
{
    return Wait(fd, cond, cond_ret, &time, state);
}

int Epoller::TimedWait(int fd, Condition cond, Condition *cond_ret, unsigned int timeout, const char* state)
{
    runtime_env::ABSTime time;
    time.Adjust(0, timeout * 1000);
    return Wait(fd, cond, cond_ret, &time, state);
}

int Epoller::Wait(int fd, Condition cond, Condition *cond_ret, const runtime_env::ABSTime *timeout, const char* state)
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    if (thread_context == nullptr)
        return -1;

    unsigned int events = 0;
    if (0 != (cond & runtime_env::ITCPSocket::C_READABLE))
        events |= EPOLLIN;
    if (0 != (cond & runtime_env::ITCPSocket::C_WRITABLE))
        events |= EPOLLOUT;

    if (0 == events)
    {
        if (cond_ret != nullptr)
            *cond_ret = runtime_env::ITCPSocket::C_NONE;
        return 0;
    }

    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    WaitingFiber wfiber;
    wfiber.fd_      = fd;
    wfiber.fiber_   = fiber;
    wfiber.events_  = events;
    wfiber.timeout_ = timeout;
    wfiber.error_   = -1;

    to_wait_fibers_lock_.Lock();
    to_wait_fibers_.push_back(&wfiber);
    thread_context->Schedule(CompleteWait, static_cast<void*>(this), state);

    if (wfiber.error_ == 0 && cond_ret != nullptr)
    {
        int cond_return = 0;
        if (0 != (wfiber.events_ & EPOLLIN))
            cond_return |= runtime_env::ITCPSocket::C_READABLE;
        if (0 != (wfiber.events_ & EPOLLOUT))
            cond_return |= runtime_env::ITCPSocket::C_WRITABLE;
        *cond_ret = static_cast<Condition>(cond_return);
    }
    return wfiber.error_;
}

void Epoller::CompleteWait(ThreadContext *thread_context, void *arg)
{
    Epoller *epoller = reinterpret_cast<Epoller*>(arg);
    epoller->to_wait_fibers_lock_.Unlock();
    epoller->RingBuzz();
}

void Epoller::EpollThreadProc(void *arg)
{
    Epoller *epoller = reinterpret_cast<Epoller*>(arg);
    if (epoller != nullptr)
        epoller->EpollThreadProc();
}

void Epoller::EpollThreadProc()
{
    log("EpollThreadProc is running");
    while (!quit_flag_)
    {
        // step 1: move all the fibers from 'to_wait_fibers_' to 'waiting_fibers_'
        to_wait_fibers_lock_.Lock();
        if (!to_wait_fibers_.empty())
        {
            for (to_wait_fibers_t::iterator iter = to_wait_fibers_.begin();
                iter != to_wait_fibers_.end(); ++iter)
            {
                WaitingFiber* const wfiber = *iter;

                epoll_event event;
                event.events = wfiber->events_;
                event.data.fd = wfiber->fd_;
                if (0 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, wfiber->fd_, &event))
                {
                    wfiber->iter_by_id_ = wfibers_by_id_.insert(
                                std::make_pair(wfiber->fd_, wfiber));
                    if (wfiber->timeout_ != nullptr)
                        wfiber->iter_by_time_ = wfibers_by_time_.insert(wfiber);
                }
                else
                {
                    // epoll_ctl failed, escalate the failure to the fiber
                    wfiber->error_ = errno;
                    env_->ReadyFiber(wfiber->fiber_);
                }
            }
            to_wait_fibers_.clear();
        }
        to_wait_fibers_lock_.Unlock();

        // step 2: wake the fibers with timed out fd
        const runtime_env::ABSTime now;
        for (waiting_fibers_by_time_t::iterator iter = wfibers_by_time_.begin();
            iter != wfibers_by_time_.end(); )
        {
            WaitingFiber *wfiber = *iter;
            assert(wfiber != nullptr);
            assert(iter == wfiber->iter_by_time_);
            if (*wfiber->timeout_ <= now)
            {
                wfiber->error_ = 0;
                wfiber->events_ = 0;
                epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, wfiber->fd_, nullptr);
                wfibers_by_id_.erase(wfiber->iter_by_id_);
                wfibers_by_time_.erase(iter++);
                env_->ReadyFiber(wfiber->fiber_);
            }
            else
                break;
        }

        // step 3: wait for events with proper timeout
        int timeout = -1;
        if (!wfibers_by_time_.empty())
        {
            WaitingFiber *wfiber = *wfibers_by_time_.begin();
            runtime_env::TimeSpan span(*wfiber->timeout_ - now);
            timeout = span.seconds() * 1000 + span.useconds() / 1000;
        }

        epoll_event events[64];
        const int count = epoll_wait(epoll_fd_, events,
                    sizeof(events)/sizeof(events[0]), timeout);
        // ignore epoll_wait error here

        // step 4: wake the fibers which associated fd has event
        for (int i=0; i<count; ++i)
        {
            const epoll_event &event = events[i];
            const int fd = event.data.fd;
            if (fd == buzz_rd_)
                ResetBuzz();
            else
            {
                for (waiting_fibers_by_id_t::iterator iter = wfibers_by_id_.lower_bound(fd);
                    iter != wfibers_by_id_.upper_bound(fd); )
                {
                    WaitingFiber *wfiber = iter->second;
                    assert(wfiber->iter_by_id_ == iter);

                    wfiber->error_ = ((event.events & wfiber->events_) != 0 ? 0 : -1);
                    wfiber->events_ = event.events;
                    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
                    if (wfiber->timeout_ != nullptr)
                        wfibers_by_time_.erase(wfiber->iter_by_time_);
                    wfibers_by_id_.erase(iter++);
                    env_->ReadyFiber(wfiber->fiber_);
                }
            }
        }
    }

    log("EpollThreadProc is quitting");
}

bool Epoller::SetupBuzz()
{
    // create buzz
    int pipefd[2];
    if (0 != pipe(pipefd))
        return false;

    buzz_rd_ = pipefd[0];
    buzz_wr_ = pipefd[1];

    if (0 != MakeNonBlock(buzz_rd_))
        goto error;
    if (0 != MakeNonBlock(buzz_wr_))
        goto error;

    // add buzz_rd_ to epoll_fd_
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = buzz_rd_;
    if (0 != epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, buzz_rd_, &event))
        goto error;

    return true;

error:
    close(buzz_rd_);
    close(buzz_wr_);
    buzz_rd_ = buzz_wr_ = -1;
    return false;
}

void Epoller::DestroyBuzz()
{
    if (epoll_fd_ >= 0 && buzz_rd_ >= 0)
    {
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, buzz_rd_, nullptr);
        close(buzz_rd_);
        close(buzz_wr_);
        buzz_rd_ = buzz_wr_ = -1;
    }
}

bool Epoller::RingBuzz()
{
    assert(buzz_wr_ >= 0);

    int ret = write(buzz_wr_, ".", 1);
    return (ret > 0 || (ret == 0 && errno == EWOULDBLOCK));
}

void Epoller::ResetBuzz()
{
    assert(buzz_rd_ >= 0);

    while (true)
    {
        unsigned char buff[1024];
        int ret = read(buzz_rd_, buff, sizeof(buff));
        if (ret <= 0)
            break;
    }
}

int Epoller::MakeNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

bool Epoller::LessWaitingFibersByTimeout::operator() (WaitingFiber* a, WaitingFiber* b)
{
    assert(a != nullptr && a->timeout_ != nullptr);
    assert(b != nullptr && b->timeout_ != nullptr);
    return *(a->timeout_) < *(b->timeout_);
}

}}  // namespace fasmio::fiber_env

