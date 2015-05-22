
#include "./cooperative-rwlock.h"

#include <assert.h>

#include "./fiber-env.h"
#include "./fiber-impl.h"

namespace fasmio { namespace fiber_env {

CooperativeRWLock::CooperativeRWLock() :
    rwlock_lock_(),
    waiting_readers_(),
    waiting_writers_(),
    readers_(0),
    writer_(nullptr)
{
}

CooperativeRWLock::~CooperativeRWLock()
{
}

bool CooperativeRWLock::ReadLock()
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    assert(thread_context != nullptr);
    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    rwlock_lock_.Lock();
    // can't rdlock if there is writer on the RWLock, nor there are waiting writers on the RWLock
    if (writer_ == nullptr && waiting_writers_.empty())
    {
        ++readers_;
        rwlock_lock_.Unlock();
        return true;
    }
    else
    {
        thread_context->Schedule(CompleteReadLock, reinterpret_cast<void*>(this), "rwlock.rdlock");
        // rwlock_lock_ will be unlock'ed in CompleteReadLock, and when this fiber
        // is re-activated, it then hold the ReadLock
        return true;
    }
}

void CooperativeRWLock::CompleteReadLock(ThreadContext *thread_context, void* arg)
{
    CooperativeRWLock *rwlock = reinterpret_cast<CooperativeRWLock*>(arg);
    assert(rwlock != nullptr);

    rwlock->waiting_readers_.push_back(thread_context->GetCurrentFiber());
    rwlock->rwlock_lock_.Unlock();
}

bool CooperativeRWLock::TryReadLock()
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    assert(thread_context != nullptr);
    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    bool locked = false;

    rwlock_lock_.Lock();
    // can't rdlock if there is writer on the RWLock, nor there are waiting writers on the RWLock
    if (writer_ == nullptr && waiting_writers_.empty())
    {
        ++readers_;
        locked = true;
    }
    rwlock_lock_.Unlock();
    return locked;
}

bool CooperativeRWLock::WriteLock()
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    assert(thread_context != nullptr);
    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    rwlock_lock_.Lock();
    if (writer_ == nullptr && readers_ == 0)
    {
        writer_ = fiber;
        rwlock_lock_.Unlock();
        return true;
    }
    else
    {
        thread_context->Schedule(CompleteWriteLock, reinterpret_cast<void*>(this), "rwlock.wrlock");
        // rwlock_lock_ will be unlock'ed in CompleteWriteLock, and when this fiber
        // is re-activated, it then hold the WriteLock
        return true;
    }
}

void CooperativeRWLock::CompleteWriteLock(ThreadContext *thread_context, void* arg)
{
    CooperativeRWLock *rwlock = reinterpret_cast<CooperativeRWLock*>(arg);
    assert(rwlock != nullptr);

    rwlock->waiting_writers_.push_back(thread_context->GetCurrentFiber());
    rwlock->rwlock_lock_.Unlock();
}

bool CooperativeRWLock::TryWriteLock()
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    assert(thread_context != nullptr);
    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    bool locked = false;

    rwlock_lock_.Lock();
    if (writer_ == nullptr && readers_ == 0)
    {
        writer_ = fiber;
        locked = true;
    }
    rwlock_lock_.Unlock();
    return locked;
}

void CooperativeRWLock::Unlock()
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    assert(thread_context != nullptr);
    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    rwlock_lock_.Lock();

    if (writer_ == nullptr)
    {
        assert(readers_ > 0);
        if (--readers_ == 0)
        {
            // wake the first waiting writer if any
            if (!waiting_writers_.empty())
            {
                FiberImpl* waiting_writer = waiting_writers_.front();
                waiting_writers_.pop_front();

                writer_ = waiting_writer;
                thread_context->ReadyFiber(waiting_writer);
            }
        }
    }
    else
    {
        assert(readers_ == 0);
        assert(writer_ == fiber);
        writer_ = nullptr;

        // wake all the waiting reader if there is any
        if (!waiting_readers_.empty())
        {
            do
            {
                FiberImpl* waiting_reader = waiting_readers_.front();
                waiting_readers_.pop_front();

                ++readers_;
                thread_context->ReadyFiber(waiting_reader);
            } while (!waiting_readers_.empty());
        }
        // otherwise wake the first waiting writer if there is any
        else if (!waiting_writers_.empty())
        {
            FiberImpl* waiting_writer = waiting_writers_.front();
            waiting_writers_.pop_front();

            writer_ = waiting_writer;
            thread_context->ReadyFiber(waiting_writer);
        }
    }

    rwlock_lock_.Unlock();
}

}}  // namespace fasmio::fiber_env

