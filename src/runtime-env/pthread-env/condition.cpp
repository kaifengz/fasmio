
#include "./condition.h"

#include "interface/runtime-env/abs-time.h"

namespace fasmio { namespace pthread_env {

Condition::Condition(Mutex *mutex) :
    mutex_(*mutex)
{
    pthread_cond_init(&cond_, nullptr);
}

Condition::~Condition()
{
    pthread_cond_destroy(&cond_);
}

bool Condition::Acquire()
{
    return mutex_.Lock();
}

bool Condition::TryAcquire()
{
    return mutex_.TryLock();
}

void Condition::Release()
{
    return mutex_.Unlock();
}

bool Condition::Wait()
{
    return 0 == pthread_cond_wait(&cond_, &mutex_.mutex_);
}

bool Condition::TimedWait(const runtime_env::ABSTime &time)
{
    struct timespec tp;
    tp.tv_sec  = time.seconds();
    tp.tv_nsec = time.useconds() * 1000;
    return 0 == pthread_cond_timedwait(&cond_, &mutex_.mutex_, &tp);
}

bool Condition::TimedWait(unsigned int timeout)
{
    runtime_env::ABSTime time;
    time.Adjust(0, timeout * 1000);
    return TimedWait(time);
}

bool Condition::Signal()
{
    return 0 == pthread_cond_signal(&cond_);
}

bool Condition::Broadcast()
{
    return 0 == pthread_cond_broadcast(&cond_);
}

}}  // namespace fasmio::pthread_env

