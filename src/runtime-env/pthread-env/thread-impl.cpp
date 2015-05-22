
#include "./thread-impl.h"
#include "./pthread-env.h"
#include <assert.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define gettid()   ((pid_t)syscall(SYS_gettid))

namespace fasmio { namespace pthread_env {

ThreadImpl::ThreadImpl(PthreadEnv* env, const char* name) :
    env_(env),
    name_(name != nullptr ? name : ""),
    thread_(nullptr),
    func_(nullptr),
    arg_(nullptr),
    id_(gettid()),
    quit_code_(0),
    thread_type_(TT_MAIN_THREAD),
    thread_state_(TS_RUNNING)
{
}

ThreadImpl::ThreadImpl(PthreadEnv *env, int (*func)(void*), void* arg, unsigned long stack_size, const char* name) :
    env_(env),
    name_(name != nullptr ? name : ""),
    thread_(new PlatformThread(ThreadEntry, this, stack_size)),
    func_(func),
    arg_(arg),
    id_(0),
    quit_code_(0),
    thread_type_(TT_NORMAL),
    thread_state_(TS_NOT_START)
{
}

ThreadImpl::~ThreadImpl()
{
    if (thread_ != nullptr)
        delete thread_;
}

int ThreadImpl::Join()
{
    if (!thread_->Join())
        return -1;

    env_->ThreadJoined(this);

    int quit_code = quit_code_;
    delete this;
    return quit_code;
}

const char* ThreadImpl::GetName()
{
    return name_.c_str();
}

unsigned int ThreadImpl::GetID()
{
    while (thread_state_ == TS_STARTING)
        sched_yield();

    return id_;
}

void ThreadImpl::SetDaemon()
{
    if (thread_type_ == TT_NORMAL)
        thread_type_ = TT_DAEMON;

    if (thread_state_ == TS_QUITTING)
        Join();
    else
        env_->AddDaemonThread(this);
}

bool ThreadImpl::IsDaemon()
{
    return thread_type_ != TT_NORMAL;
}

bool ThreadImpl::Start()
{
    if (!thread_->Start())
        return false;

    thread_state_ = TS_STARTING;
    return true;
}

void ThreadImpl::CurrentThread(ThreadImpl* thread)
{
    PlatformThread::SetSpecific(reinterpret_cast<void*>(thread));
}

ThreadImpl* ThreadImpl::CurrentThread()
{
    return reinterpret_cast<ThreadImpl*>(PlatformThread::GetSpecific());
}

void ThreadImpl::ThreadEntry(void* thr)
{
    ThreadImpl *thread = reinterpret_cast<ThreadImpl*>(thr);
    assert(thread != nullptr);
    thread->id_ = gettid();
    thread->thread_state_ = TS_RUNNING;

    if (thread->func_ != nullptr)
    {
        CurrentThread(thread);
        thread->quit_code_ = (thread->func_)(thread->arg_);
        CurrentThread(nullptr);
    }

    thread->thread_state_ = TS_QUITTING;
}

}}  // namespace fasmio::pthread_env

