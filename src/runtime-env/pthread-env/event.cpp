
#include "./event.h"

namespace fasmio { namespace pthread_env {

Event::Event(bool init_state) :
    mutex_(false),
    cond_(&mutex_),
    flag_(init_state)
{
}

Event::~Event()
{
}

bool Event::Wait()
{
    mutex_.Lock();
    if (!flag_)
        cond_.Wait();
    mutex_.Unlock();
    return true;
}

bool Event::TimedWait(const runtime_env::ABSTime &time)
{
    bool ret = true;
    mutex_.Lock();
    if (!flag_)
        ret = cond_.TimedWait(time);
    mutex_.Unlock();
    return ret;
}

bool Event::TimedWait(unsigned int timeout)
{
    bool ret = true;
    mutex_.Lock();
    if (!flag_)
        ret = cond_.TimedWait(timeout);
    mutex_.Unlock();
    return ret;
}

bool Event::Set()
{
    mutex_.Lock();
    if (!flag_)
    {
        flag_ = true;
        cond_.Broadcast();
    }
    mutex_.Unlock();
    return true;
}

bool Event::Reset()
{
    flag_ = false;
    return true;
}

bool Event::Pulse()
{
    mutex_.Lock();
    if (!flag_)
        cond_.Broadcast();
    mutex_.Unlock();
    return true;
}

bool Event::IsSet()
{
    return flag_;
}

}}  // namespace fasmio::pthread_env

