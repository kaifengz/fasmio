
#include "./pthread-env.h"

#include <assert.h>
#include <sched.h>
#include <unistd.h>

#include <memory>

#include "./condition.h"
#include "./event.h"
#include "./file.h"
#include "./mutex.h"
#include "./rwlock.h"
#include "./semaphore.h"
#include "./socket.h"
#include "./thread-impl.h"

namespace fasmio { namespace pthread_env {

PthreadEnv::PthreadEnv() :
    threads_(),
    daemon_threads_(),
    threads_lock_(false),
    threads_quit_cond_(&threads_lock_)
{
}

PthreadEnv::~PthreadEnv()
{
}

runtime_env::ITCPSocket* PthreadEnv::NewTCPSocket()
{
    return new TCPSocket();
}

runtime_env::IFile* PthreadEnv::NewFile()
{
    return new File();
}

runtime_env::IMutex* PthreadEnv::NewMutex(bool recursive)
{
    return new Mutex(recursive);
}

runtime_env::IRWLock* PthreadEnv::NewRWLock()
{
    return new RWLock();
}

runtime_env::ICondition* PthreadEnv::NewCondition(runtime_env::IMutex *mutex)
{
    if (mutex == nullptr)
        return nullptr;

    Mutex* pthread_mutex = dynamic_cast<Mutex*>(mutex);
    if (pthread_mutex == nullptr)
        return nullptr;

    return new Condition(pthread_mutex);
}

runtime_env::IEvent* PthreadEnv::NewEvent(bool init_state)
{
    return new Event(init_state);
}

runtime_env::ISemaphore* PthreadEnv::NewSemaphore(unsigned int max_count, unsigned int init_count)
{
    if (max_count == 0)
        return nullptr;
    if (max_count < init_count)
        return nullptr;

    return new Semaphore(max_count, init_count);
}

void PthreadEnv::Sleep(unsigned long milliseconds)
{
    usleep(milliseconds * 1000);
}

void PthreadEnv::Yield()
{
    sched_yield();
}

void PthreadEnv::Quit(int quit_code)
{
    pthread_exit(reinterpret_cast<void*>(quit_code));
}

runtime_env::IThread* PthreadEnv::CreateThread(int (*func)(void*), void* arg, const char* name)
{
    if (func == nullptr)
        return nullptr;

    std::unique_ptr<ThreadImpl> thread(new ThreadImpl(this, func, arg, 0, name));
    if (thread.get() == nullptr)
        return nullptr;
    if (!thread->Start())
        return nullptr;

    threads_lock_.Lock();
        threads_.insert(thread.get());
    threads_lock_.Unlock();

    return thread.release();
}

runtime_env::IThread* PthreadEnv::CurrentThread()
{
    return static_cast<runtime_env::IThread*>(ThreadImpl::CurrentThread());
}

int PthreadEnv::RunMain(int (*main_proc)(IRuntimeEnv*, int, char*[]), int argc, char* argv[])
{
    ThreadImpl main_thread(this, "main");
    ThreadImpl::CurrentThread(&main_thread);
    int ret = main_proc(this, argc, argv);
    ThreadImpl::CurrentThread(nullptr);

    WaitAllThreads();

    return ret;
}

void PthreadEnv::ThreadJoined(ThreadImpl *thread)
{
    threads_lock_.Lock();
        threads_.erase(thread);
        if (threads_.empty() && daemon_threads_.empty())
            threads_quit_cond_.Signal();
    threads_lock_.Unlock();
}

bool PthreadEnv::AddDaemonThread(ThreadImpl *thread)
{
    threads_lock_.Lock();
        JoinDaemonThreads_nolock();
        threads_.erase(thread);
        daemon_threads_.insert(thread);
    threads_lock_.Unlock();

    return true;
}

bool PthreadEnv::WaitAllThreads()
{
    threads_lock_.Lock();
    while (true)
    {
        JoinDaemonThreads_nolock();
        if (threads_.empty() && daemon_threads_.empty())
            break;
        threads_quit_cond_.TimedWait(100);
    }
    threads_lock_.Unlock();

    return 0;
}

unsigned int PthreadEnv::JoinDaemonThreads_nolock()
{
    unsigned int joined = 0;
    for (threads_t::iterator iter = daemon_threads_.begin();
        iter != daemon_threads_.end(); )
    {
        ThreadImpl *thread = *iter;
        assert(thread != nullptr);
        if (thread->thread_state_ == ThreadImpl::TS_QUITTING)
        {
            thread->Join();
            daemon_threads_.erase(iter++);
            ++joined;
        }
        else
            ++iter;
    }

    return joined;
}

}}  // namespace fasmio::pthread_env

