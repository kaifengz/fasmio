
#include "./native-mutex.h"

namespace fasmio { namespace fiber_env {

NativeMutex::NativeMutex()
{
    pthread_mutex_init(&mutex_, nullptr);
}

NativeMutex::~NativeMutex()
{
    pthread_mutex_destroy(&mutex_);
}

bool NativeMutex::Lock()
{
    return 0 == pthread_mutex_lock(&mutex_);
}

bool NativeMutex::TryLock()
{
    return 0 == pthread_mutex_trylock(&mutex_);
}

void NativeMutex::Unlock()
{
    pthread_mutex_unlock(&mutex_);
}

}}  // namespace fasmio::fiber_env

