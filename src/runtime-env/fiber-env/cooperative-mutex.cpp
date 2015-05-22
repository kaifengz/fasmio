
#include "./cooperative-mutex.h"

#include <assert.h>

#include "./fiber-env.h"
#include "./fiber-impl.h"

namespace fasmio { namespace fiber_env {

CooperativeMutex::CooperativeMutex(bool recursive) :
    recursive_(recursive),
    lock_lock_(),
    locked_(0),
    owner_(nullptr),
    waiting_queue_()
{
}

CooperativeMutex::~CooperativeMutex()
{
}

bool CooperativeMutex::Lock()
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    if (thread_context == nullptr)
        return false;

    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    lock_lock_.Lock();
    if (locked_ == 0)
    {
        ++locked_;
        owner_ = fiber;
    }
    else
    {
        if (recursive_ && owner_ == fiber)
            ++locked_;
        else
        {
            thread_context->Schedule(CompleteLock, reinterpret_cast<void*>(this), "mutex.lock");
            // lock_lock_ will be unlock'ed in CompleteLock, and when this fiber
            // is re-activated, it then hold the CooperativeMutex
            return true;
        }
    }
    lock_lock_.Unlock();
    return true;
}

void CooperativeMutex::CompleteLock(ThreadContext *thread_context, void* arg)
{
    CooperativeMutex *mutex = reinterpret_cast<CooperativeMutex*>(arg);
    assert(mutex != nullptr);

    mutex->waiting_queue_.push_back(thread_context->GetCurrentFiber());
    mutex->lock_lock_.Unlock();
}

bool CooperativeMutex::TryLock()
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    if (thread_context == nullptr)
        return false;

    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    bool locked = false;

    lock_lock_.Lock();
    if (locked_ == 0)
    {
        ++locked_;
        owner_ = fiber;
        locked = true;
    }
    else if (recursive_ && owner_ == fiber)
    {
        ++locked_;
        locked = true;
    }
    lock_lock_.Unlock();

    return locked;
}

void CooperativeMutex::Unlock()
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    if (thread_context == nullptr)
        return;

    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    lock_lock_.Lock();
    assert(locked_ > 0);
    assert(owner_ == fiber);

    if (--locked_ == 0)
    {
        if (waiting_queue_.empty())
            owner_ = nullptr;
        else
        {
            FiberImpl* waiting_fiber = waiting_queue_.front();
            waiting_queue_.pop_front();

            ++locked_;
            owner_ = waiting_fiber;

            thread_context->ReadyFiber(waiting_fiber);
        }
    }
    lock_lock_.Unlock();
}

}}  // namespace fasmio::fiber_env

