
#include "./cooperative-condition.h"

#include <assert.h>

#include "./fiber-impl.h"

namespace fasmio { namespace fiber_env {

CooperativeCondition::CooperativeCondition(CooperativeMutex *mutex) :
    mutex_(*mutex),
    waitable_()
{
}

CooperativeCondition::~CooperativeCondition()
{
}

bool CooperativeCondition::Acquire()
{
    return mutex_.Lock();
}

bool CooperativeCondition::TryAcquire()
{
    return mutex_.TryLock();
}

void CooperativeCondition::Release()
{
    mutex_.Unlock();
}

bool CooperativeCondition::Wait()
{
    // mutex_.Unlock will be called in CompleteWait
    waitable_.Wait("condition.wait", CompleteWait, static_cast<void*>(this));

    // when this fiber is re-activated, we should re-acquire the lock
    mutex_.Lock();
    return true;
}

bool CooperativeCondition::TimedWait(const runtime_env::ABSTime &time)
{
    // mutex_.Unlock will be called in CompleteWait
    bool wait_succeed = waitable_.TimedWait(time, "condition.timedwait",
                CompleteWait, static_cast<void*>(this));

    // when this fiber is re-activated, we should re-acquire the lock
    mutex_.Lock();
    return wait_succeed;
}

bool CooperativeCondition::TimedWait(unsigned int timeout)
{
    // mutex_.Unlock will be called in CompleteWait
    bool wait_succeed = waitable_.TimedWait(timeout, "condition.timedwait",
                CompleteWait, static_cast<void*>(this));

    // when this fiber is re-activated, we should re-acquire the lock
    mutex_.Lock();
    return wait_succeed;
}

void CooperativeCondition::CompleteWait(void* arg)
{
    CooperativeCondition* cond = reinterpret_cast<CooperativeCondition*>(arg);
    assert(cond != nullptr);

    cond->mutex_.Unlock();
}

bool CooperativeCondition::Signal()
{
    waitable_.Signal(1);
    return true;
}

bool CooperativeCondition::Broadcast()
{
    waitable_.Broadcast();
    return true;
}

}}  // namespace fasmio::fiber_env

