
#include "./fiber-env.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "./cooperative-condition.h"
#include "./cooperative-event.h"
#include "./cooperative-file.h"
#include "./cooperative-mutex.h"
#include "./cooperative-rwlock.h"
#include "./cooperative-semaphore.h"
#include "./cooperative-socket.h"
#include "./log.h"
#include "runtime-env/interlocked.h"

namespace fasmio { namespace fiber_env {

static const char* const g_join    = "join";
static const char* const g_quitted = "quitted";
static const char* const g_sleep   = "sleep";

FiberEnv::FiberEnv() :
    quit_flag_(false),
    live_fibers_(),
    quitted_fibers_(),
    joining_fibers_(),
    fibers_lock_(),
    no_live_fibers_cond_(&fibers_lock_),
    ready_queue_(),
    threads_(),
    fiberized_threads_(0),
    waiting_fiber_manager_(),
    epoller_()
{
}

FiberEnv::~FiberEnv()
{
}

runtime_env::ITCPSocket* FiberEnv::NewTCPSocket()
{
    if (FiberEnv::GetThreadContext() == nullptr)
        return nullptr;

    return new CooperativeTCPSocket();
}

runtime_env::IFile* FiberEnv::NewFile()
{
    if (FiberEnv::GetThreadContext() == nullptr)
        return nullptr;

    return new CooperativeFile();
}

runtime_env::IMutex* FiberEnv::NewMutex(bool recursive)
{
    if (FiberEnv::GetThreadContext() == nullptr)
        return nullptr;

    return new CooperativeMutex(recursive);
}

runtime_env::IRWLock* FiberEnv::NewRWLock()
{
    if (FiberEnv::GetThreadContext() == nullptr)
        return nullptr;

    return new CooperativeRWLock();
}

runtime_env::ICondition* FiberEnv::NewCondition(runtime_env::IMutex *mutex)
{
    if (mutex == nullptr)
        return nullptr;
    if (FiberEnv::GetThreadContext() == nullptr)
        return nullptr;

    CooperativeMutex *cooperative_mutex = dynamic_cast<CooperativeMutex*>(mutex);
    if (cooperative_mutex == nullptr)
        return nullptr;

    return new CooperativeCondition(cooperative_mutex);
}

runtime_env::IEvent* FiberEnv::NewEvent(bool init_state)
{
    if (FiberEnv::GetThreadContext() == nullptr)
        return nullptr;

    return new CooperativeEvent(init_state);
}

runtime_env::ISemaphore* FiberEnv::NewSemaphore(unsigned int max_count, unsigned int init_count)
{
    if (max_count == 0)
        return nullptr;
    if (max_count < init_count)
        return nullptr;
    if (FiberEnv::GetThreadContext() == nullptr)
        return nullptr;

    return new CooperativeSemaphore(max_count, init_count);
}

bool FiberEnv::Start(unsigned int threads)
{
    if (!waiting_fiber_manager_.Start(this))
        return false;
    if (!epoller_.Start(this))
        return false;

    const unsigned long kThreadStackSize = 256 * 1024;

    for (unsigned int i = 0; i < threads; ++i)
        threads_.push_back(new PlatformThread(ThreadProc, static_cast<void*>(this), kThreadStackSize));
    for (unsigned int i = 0; i < threads; ++i)
        threads_[i]->Start();

    return true;
}

void FiberEnv::Stop()
{
    quit_flag_ = true;
}

bool FiberEnv::Join()
{
    ThreadContext *thread_context = GetThreadContext();
    if (thread_context != nullptr)
    {
        // cannot call Stop inside a fiber, otherwise would cause a deadlock
        return false;
    }

    if (!quit_flag_)
        FiberizeThisThread();

    const unsigned int thread_count = threads_.size();
    for (unsigned int i = 0; i < thread_count; ++i)
        threads_[i]->Join();
    for (unsigned int i = 0; i < thread_count; ++i)
        delete threads_[i];
    threads_.clear();

    while (fiberized_threads_ > 0)
        usleep(1000);

    // should no living fibers right now
    assert(live_fibers_.empty());

    // cleanup all the quitted fibers, lock is innecessary here
    for (fibers_t::iterator iter = quitted_fibers_.begin();
        iter != quitted_fibers_.end(); ++iter)
    {
        FiberImpl* fiber = iter->second;
        delete fiber;
    }
    quitted_fibers_.clear();

    epoller_.Stop();
    waiting_fiber_manager_.Stop();
    return true;
}

void FiberEnv::RunConsole()
{
    while (true)
    {
        switch (getchar())
        {
        case 'd':
            DumpFiberStates();
            break;

        case 'q':
            return;

        case 'j':
            Join();
            break;

        case '?':
        default:
            break;
        }
    }
}

void FiberEnv::DumpFiberStates()
{
    fibers_lock_.Lock();
    printf("%d schedule thread(s), ready-queue length %d\n", threads_.size(), ready_queue_.Size());
    printf("%d living fiber(s):\n", live_fibers_.size());
    for (fibers_t::iterator iter = live_fibers_.begin();
        iter != live_fibers_.end(); ++iter)
    {
        iter->second->Dump(stdout);
    }

    printf("%d zombie fiber(s):\n", quitted_fibers_.size());
    for (fibers_t::iterator iter = quitted_fibers_.begin();
        iter != quitted_fibers_.end(); ++iter)
    {
        iter->second->Dump(stdout);
    }

    fibers_lock_.Unlock();
}

runtime_env::IThread* FiberEnv::CreateThread(int (*func)(void*), void* arg, const char* name)
{
    FiberImpl* fiber = new FiberImpl(this, func, arg, 0, name);

    fibers_lock_.Lock();
    live_fibers_[fiber->id_] = fiber;
    fibers_lock_.Unlock();

    ReadyFiber(fiber);
    return static_cast<runtime_env::IThread*>(fiber);
}

runtime_env::IThread* FiberEnv::CreateThread_s(int (*func)(void*), void* arg, const char* name)
{
    ThreadContext* thread_context = GetThreadContext();
    if (thread_context == nullptr)
        return nullptr;

    FiberEnv* env = thread_context->env_;
    assert(env != nullptr);

    return env->CreateThread(func, arg, name);
}

bool FiberEnv::FiberizeThisThread()
{
    ThreadContext *thread_context = GetThreadContext();
    if (thread_context != nullptr)
    {
        // already inside a fiber
        return false;
    }

    runtime_env::interlocked::Increment((long*)&fiberized_threads_);
    ThreadProc();
    runtime_env::interlocked::Decrement((long*)&fiberized_threads_);
    return true;
}

void FiberEnv::Sleep(unsigned long milliseconds)
{
    Sleep_s(milliseconds);
}

void FiberEnv::Sleep_s(unsigned long milliseconds)
{
    ThreadContext *thread_context = GetThreadContext();
    if (thread_context == nullptr)
        return;

    FiberImpl* fiber = thread_context->current_fiber_;
    assert(fiber != nullptr);

    if (milliseconds == 0)
    {
        Yield_s();
        return;
    }

    runtime_env::ABSTime timeout;
    timeout.Adjust(0, milliseconds * 1000);

    WaitingFiberManager::WaitingFiber wfiber;
    wfiber.fiber_   = fiber;
    wfiber.state_   = WaitingFiberManager::STATE_SLEEPING;
    wfiber.timeout_ = timeout;
    thread_context->Schedule(CompleteSleep, static_cast<void*>(&wfiber), g_sleep);

    assert(wfiber.reason_ == WaitingFiberManager::REASON_TIMEDOUT);
}

void FiberEnv::CompleteSleep(ThreadContext* thread_context, void* arg)
{
    WaitingFiberManager::WaitingFiber *wfiber =
            reinterpret_cast<WaitingFiberManager::WaitingFiber*>(arg);
    assert(wfiber != nullptr);
    assert(wfiber->state_ == WaitingFiberManager::STATE_SLEEPING);

    WaitingFiberManager *manager = thread_context->GetWaitingFiberManager();
    assert (manager != nullptr);
    manager->AddWaitingFiber(wfiber);
}

void FiberEnv::Yield()
{
    Yield_s();
}

void FiberEnv::Yield_s()
{
    ThreadContext *thread_context = GetThreadContext();
    if (thread_context == nullptr)
        return;

    FiberEnv *env = thread_context->env_;
    unsigned int ready_queue_size = env->ready_queue_.Size();
    if (ready_queue_size == 0)
        // no other ready fiber, continue current one
        return;

    thread_context->Schedule(CompleteYield, nullptr, nullptr);
}

void FiberEnv::CompleteYield(ThreadContext* thread_context, void*)
{
    thread_context->env_->ReadyFiber(thread_context->current_fiber_);
}

void FiberEnv::Quit(int quit_code)
{
    Quit_s(quit_code);
}

void FiberEnv::Quit_s(int quit_code)
{
    ThreadContext *thread_context = GetThreadContext();
    if (thread_context == nullptr)
        return;

    FiberImpl* fiber = thread_context->current_fiber_;
    assert(fiber != nullptr);
    fiber->SetQuitCode(quit_code);

    thread_context->Schedule(CompleteQuit, nullptr, g_quitted);
}

void FiberEnv::CompleteQuit(ThreadContext* thread_context, void*)
{
    FiberEnv *env = thread_context->env_;
    FiberImpl *fiber = thread_context->current_fiber_;

    fiber->FreeResource();

    env->fibers_lock_.Lock();
    {
        // remove the fiber from live-fibers
        env->live_fibers_.erase(fiber->id_);
        if (env->live_fibers_.empty())
            env->no_live_fibers_cond_.Broadcast();

        // check whether someone is joining this fiber
        fibers_t::iterator iter = env->joining_fibers_.find(fiber->id_);
        if (iter != env->joining_fibers_.end())
        {
            // Don't add the fiber to quitted-fibers if someone is joining it,
            // but instead wake up the joining fiber.
            // The fiber object will be deleted in the Join function
            env->ReadyFiber(iter->second);
            env->joining_fibers_.erase(iter);
        }
        else if (fiber->daemon_)
        {
            // It's a daemon fiber and no one if is joining it, delete the
            // object directly
            delete fiber;
        }
        else
        {
            // add the fiber to quitted-fibers if no one is joining it
            env->quitted_fibers_[fiber->id_] = fiber;
        }
    }
    env->fibers_lock_.Unlock();
}

void FiberEnv::Stop_s()
{
    ThreadContext* thread_context = GetThreadContext();
    if (thread_context == nullptr)
        return;

    FiberEnv* env = thread_context->env_;
    assert(env != nullptr);

    env->Stop();
}

runtime_env::IThread* FiberEnv::CurrentThread()
{
    return CurrentThread_s();
}

runtime_env::IThread* FiberEnv::CurrentThread_s()
{
    ThreadContext* thread_context = GetThreadContext();
    if (thread_context == nullptr)
        return nullptr;

    assert(thread_context->current_fiber_ != nullptr);
    return static_cast<runtime_env::IThread*>(thread_context->current_fiber_);
}

struct RunMain_args
{
    int (*main)(IRuntimeEnv*, int, char*[]);
    FiberEnv* env;
    int argc;
    char** argv;
    int ret;
};

int FiberEnv::RunMain(int (*fiber_main)(IRuntimeEnv*, int, char*[]), int argc, char* argv[])
{
    if (fiber_main == nullptr)
        return 1;

    ThreadContext *thread_context = GetThreadContext();
    if (thread_context != nullptr)
        // can not call RunMain from fiber
        return 1;

    RunMain_args args;
    args.main = fiber_main;
    args.env = this;
    args.argc = argc;
    args.argv = argv;
    args.ret = 0;

    runtime_env::IThread *main = CreateThread(FiberMainWrapper, reinterpret_cast<void*>(&args), "main");
    if (main == nullptr)
        return 1;

    const unsigned int kThreadCount = 8;
    if (!Start(kThreadCount))
        return 1;

    if (!FiberizeThisThread())
        return 1;

    Join();
    return args.ret;
}

int FiberEnv::FiberMainWrapper(void* arg)
{
    RunMain_args *args = reinterpret_cast<RunMain_args*>(arg);
    assert(args != nullptr);

    args->ret = (args->main)(args->env, args->argc, args->argv);
    args->env->Stop();
    return 0;
}

int FiberEnv::JoinFiber_s(FiberImpl* fiber)
{
    if (fiber == nullptr)
        return -1;

    ThreadContext *thread_context = GetThreadContext();
    if (thread_context == nullptr)
        return -1;

    FiberEnv *env = thread_context->env_;
    assert(env != nullptr);

    env->fibers_lock_.Lock();

    fibers_t::iterator iter = env->quitted_fibers_.find(fiber->id_);
    if (iter == env->quitted_fibers_.end())
    {
        thread_context->Schedule(CompleteJoinFiber, static_cast<void*>(fiber), g_join);
        // upon the return of thread_context->Schedule,
        // fibers_lock_ will be Unlock'ed
    }
    else
    {
        assert(fiber == iter->second);
        env->quitted_fibers_.erase(iter);
        env->fibers_lock_.Unlock();
    }

    int fiber_quit_code = fiber->quit_code_;
    delete fiber;
    return fiber_quit_code;
}

void FiberEnv::CompleteJoinFiber(ThreadContext* thread_context, void* arg)
{
    FiberImpl *fiber = reinterpret_cast<FiberImpl*>(arg);
    assert(fiber != nullptr);

    FiberImpl *joining_fiber = thread_context->current_fiber_;
    assert(joining_fiber != nullptr);

    FiberEnv *env = thread_context->env_;
    assert(env != nullptr);

    env->joining_fibers_[fiber->id_] = joining_fiber;
    env->fibers_lock_.Unlock();
}

void FiberEnv::SetCurrentFiberState(const char* state)
{
    ThreadContext* thread_context = GetThreadContext();
    assert(thread_context != nullptr);
    thread_context->current_fiber_->SetState(state);
}

void FiberEnv::SetCurrentFiberStateFromRunning(const char* state)
{
    ThreadContext* thread_context = GetThreadContext();
    assert(thread_context != nullptr);
    thread_context->current_fiber_->SetStateFromRunning(state);
}

/////////////////////////////////////////////////////////////////////////////

void FiberEnv::ReadyFiber(FiberImpl* fiber)
{
    assert(fiber != nullptr);
    fiber->SetStateToReady();
    fiber->ChangeStatus(FiberImpl::FS_READY);
    ready_queue_.Push(fiber);
}

void FiberEnv::RecycleFiberIfQuitted(FiberImpl* fiber)
{
    fibers_lock_.Lock();

    fibers_t::iterator iter = quitted_fibers_.find(fiber->id_);
    if (iter != quitted_fibers_.end())
    {
        assert(iter->second == fiber);
        quitted_fibers_.erase(iter);
        delete fiber;
    }

    fibers_lock_.Unlock();
}

void FiberEnv::ThreadProc(void* arg)
{
    FiberEnv* env = reinterpret_cast<FiberEnv*>(arg);
    if (env != nullptr)
        env->ThreadProc();
}

void FiberEnv::ThreadProc()
{
    PlatformFiber this_fiber;

    ThreadContext thread_context;
    thread_context.env_                = this;
    thread_context.schedule_fiber_     = &this_fiber;
    thread_context.current_fiber_      = nullptr;
    thread_context.next_fiber_         = nullptr;
    thread_context.post_switch_action_ = nullptr;
    thread_context.post_switch_arg_    = nullptr;

    PlatformThread::SetSpecific(&thread_context);

    while (true)
    {
        thread_context.next_fiber_ = ready_queue_.Pop(true, 10);
        if (thread_context.next_fiber_ == nullptr)
        {
            // quit the schedule thread only if no living fibers
            if (quit_flag_)
            {
                fibers_lock_.Lock();
                bool no_living_fibers = live_fibers_.empty();
                fibers_lock_.Unlock();
                if (no_living_fibers)
                    break;
            }

            continue;
        }

        thread_context.next_fiber_->ChangeStatus(FiberImpl::FS_RUNNING);
        thread_context.next_fiber_->SetStateToRunning();
        thread_context.next_fiber_->fiber_->SwitchTo(&this_fiber);

        if (thread_context.post_switch_action_ != nullptr)
        {
            (thread_context.post_switch_action_)(&thread_context, thread_context.post_switch_arg_);
            thread_context.post_switch_action_ = nullptr;
        }
    }

    PlatformThread::SetSpecific(nullptr);
}

bool FiberEnv::Schedule(ThreadContext *thread_context, void (*func)(ThreadContext*, void*), void*arg, const char* state)
{
    thread_context->current_fiber_->ChangeStatus(FiberImpl::FS_WAITING);

    if (state != nullptr)
        thread_context->current_fiber_->SetStateFromRunning(state);

    thread_context->post_switch_action_ = func;
    thread_context->post_switch_arg_    = arg;

    thread_context->next_fiber_ = ready_queue_.Pop(false);
    if (thread_context->next_fiber_ != nullptr)
    {
        thread_context->next_fiber_->ChangeStatus(FiberImpl::FS_RUNNING);
        thread_context->next_fiber_->SetStateToRunning();
        if (!thread_context->next_fiber_->fiber_->SwitchTo(thread_context->current_fiber_->fiber_))
            return false;
    }
    else
    {
        if (!thread_context->schedule_fiber_->SwitchTo(thread_context->current_fiber_->fiber_))
            return false;
    }

    // after SwitchTo we may runs in another thread-context now!
    thread_context = GetThreadContext();
    assert(thread_context != nullptr);
    assert(thread_context->next_fiber_ != nullptr);

    if (thread_context->post_switch_action_ != nullptr)
    {
        thread_context->post_switch_action_(thread_context, thread_context->post_switch_arg_);
        thread_context->post_switch_action_ = nullptr;
    }

    thread_context->current_fiber_ = thread_context->next_fiber_;
    return true;
}

void FiberEnv::InitFiberSchedule(ThreadContext *thread_context)
{
    if (thread_context->post_switch_action_ != nullptr)
    {
        thread_context->post_switch_action_(thread_context, thread_context->post_switch_arg_);
        thread_context->post_switch_action_ = nullptr;
    }

    assert(thread_context->next_fiber_ != nullptr);
    thread_context->current_fiber_ = thread_context->next_fiber_;
}

ThreadContext* FiberEnv::GetThreadContext()
{
    return reinterpret_cast<ThreadContext*>(PlatformThread::GetSpecific());
}

FiberEnv* FiberEnv::GetEnv()
{
    ThreadContext* thread_context = GetThreadContext();
    return thread_context != nullptr ? thread_context->env_ : nullptr;
}


/////////////////////////////////////////////////////////////////////////////

FiberImpl* ThreadContext::GetCurrentFiber()
{
    return current_fiber_;
}

void ThreadContext::ReadyFiber(FiberImpl* fiber)
{
    env_->ReadyFiber(fiber);
}

bool ThreadContext::Schedule(const char* state)
{
    return env_->Schedule(this, nullptr, nullptr, state);
}

bool ThreadContext::Schedule(void (*func)(ThreadContext*, void*), void* arg, const char* state)
{
    return env_->Schedule(this, func, arg, state);
}

void ThreadContext::InitFiberSchedule()
{
    return env_->InitFiberSchedule(this);
}

WaitingFiberManager* ThreadContext::GetWaitingFiberManager()
{
    return &env_->waiting_fiber_manager_;
}

Epoller* ThreadContext::GetEpoller()
{
    return &env_->epoller_;
}

}}  // namespace fasmio::fiber_env

