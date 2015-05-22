
#include "./cooperative-semaphore.h"

#include <assert.h>

namespace fasmio { namespace fiber_env {

CooperativeSemaphore::CooperativeSemaphore(unsigned int max_count, unsigned int init_count) :
    lock_(),
    max_count_(max_count),
    count_(init_count),
    waitable_()
{
    assert(max_count > 0);
    assert(max_count >= init_count);
}

CooperativeSemaphore::~CooperativeSemaphore()
{
}

bool CooperativeSemaphore::Wait()
{
    lock_.Lock();
    if (count_ > 0)
    {
        --count_;
        lock_.Unlock();
        return true;
    }
    else
    {
        assert(count_ == 0);
        return waitable_.Wait("semaphore.wait", CompleteWait, static_cast<void*>(this));
    }
}

bool CooperativeSemaphore::TimedWait(const runtime_env::ABSTime &time)
{
    lock_.Lock();
    if (count_ > 0)
    {
        --count_;
        lock_.Unlock();
        return true;
    }
    else
    {
        assert(count_ == 0);
        return waitable_.TimedWait(time, "semaphore.timedwait", CompleteWait, static_cast<void*>(this));
    }
}

bool CooperativeSemaphore::TimedWait(unsigned int timeout)
{
    lock_.Lock();
    if (count_ > 0)
    {
        --count_;
        lock_.Unlock();
        return true;
    }
    else
    {
        assert(count_ == 0);
        return waitable_.TimedWait(timeout, "semaphore.timedwait", CompleteWait, static_cast<void*>(this));
    }
}

void CooperativeSemaphore::CompleteWait(void* arg)
{
    CooperativeSemaphore *semaphore = reinterpret_cast<CooperativeSemaphore*>(arg);
    semaphore->lock_.Unlock();
}

bool CooperativeSemaphore::Release()
{
    lock_.Lock();
    assert(count_ >= 0);
    assert(count_ < max_count_);
    if (count_ == 0)
    {
        if (1 == waitable_.Signal(1))
            ;
        else
            ++count_;
    }
    else
        ++count_;
    lock_.Unlock();
    return true;
}

}}  // namespace fasmio::fiber_env

