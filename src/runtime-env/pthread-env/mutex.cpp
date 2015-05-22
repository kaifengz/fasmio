
#include "./mutex.h"

namespace fasmio { namespace pthread_env {

Mutex::Mutex(bool recursive)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex_, &attr);
    pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mutex_);
}

bool Mutex::Lock()
{
    return 0 == pthread_mutex_lock(&mutex_);
}

bool Mutex::TryLock()
{
    return 0 == pthread_mutex_trylock(&mutex_);
}

void Mutex::Unlock()
{
    pthread_mutex_unlock(&mutex_);
}

}}  // namespace fasmio::pthread_env

