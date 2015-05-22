
#include "./native-condition.h"

#include "interface/runtime-env/abs-time.h"

namespace fasmio { namespace fiber_env {

NativeCondition::NativeCondition(NativeMutex *mutex) :
    mutex_(*mutex)
{
    pthread_cond_init(&cond_, nullptr);
}

NativeCondition::~NativeCondition()
{
    pthread_cond_destroy(&cond_);
}

bool NativeCondition::Acquire()
{
    return mutex_.Lock();
}

bool NativeCondition::TryAcquire()
{
    return mutex_.TryLock();
}

void NativeCondition::Release()
{
    return mutex_.Unlock();
}

bool NativeCondition::Wait()
{
    return 0 == pthread_cond_wait(&cond_, &mutex_.mutex_);
}

bool NativeCondition::TimedWait(const runtime_env::ABSTime &time)
{
    struct timespec tp;
    tp.tv_sec  = time.seconds();
    tp.tv_nsec = time.useconds() * 1000;
    return 0 == pthread_cond_timedwait(&cond_, &mutex_.mutex_, &tp);
}

bool NativeCondition::TimedWait(unsigned int timeout)
{
    runtime_env::ABSTime time;
    time.Adjust(0, timeout * 1000);
    return TimedWait(time);
}

bool NativeCondition::Signal()
{
    return 0 == pthread_cond_signal(&cond_);
}

bool NativeCondition::Broadcast()
{
    return 0 == pthread_cond_broadcast(&cond_);
}

}}  // namespace fasmio::fiber_env

