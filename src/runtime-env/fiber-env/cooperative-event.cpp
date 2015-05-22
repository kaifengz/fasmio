
#include "./cooperative-event.h"

#include <assert.h>

namespace fasmio { namespace fiber_env {

CooperativeEvent::CooperativeEvent(bool init_state) :
    lock_(),
    flag_(init_state),
    waitable_()
{
}

CooperativeEvent::~CooperativeEvent()
{
}

bool CooperativeEvent::Wait()
{
    lock_.Lock();
    if (flag_)
    {
        lock_.Unlock();
        return true;
    }
    else
    {
        return waitable_.Wait("event.wait", CompleteWait, static_cast<void*>(this));
    }
}

bool CooperativeEvent::TimedWait(const runtime_env::ABSTime &time)
{
    lock_.Lock();
    if (flag_)
    {
        lock_.Unlock();
        return true;
    }
    else
    {
        return waitable_.TimedWait(time, "event.timedwait", CompleteWait, static_cast<void*>(this));
    }
}

bool CooperativeEvent::TimedWait(unsigned int timeout)
{
    lock_.Lock();
    if (flag_)
    {
        lock_.Unlock();
        return true;
    }
    else
    {
        return waitable_.TimedWait(timeout, "event.timedwait", CompleteWait, static_cast<void*>(this));
    }
}

void CooperativeEvent::CompleteWait(void* arg)
{
    CooperativeEvent *event = reinterpret_cast<CooperativeEvent*>(arg);
    event->lock_.Unlock();
}

bool CooperativeEvent::Set()
{
    lock_.Lock();
    if (!flag_)
    {
        flag_ = true;
        waitable_.Broadcast();
    }
    lock_.Unlock();

    return true;
}

bool CooperativeEvent::Reset()
{
    flag_ = false;
    return true;
}

bool CooperativeEvent::Pulse()
{
    lock_.Lock();
    if (!flag_)
        waitable_.Broadcast();
    lock_.Unlock();

    return true;
}

bool CooperativeEvent::IsSet()
{
    return flag_;
}

}}  // namespace fasmio::fiber_env

