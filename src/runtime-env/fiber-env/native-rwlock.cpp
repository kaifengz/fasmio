
#include "./native-rwlock.h"

namespace fasmio { namespace fiber_env {

NativeRWLock::NativeRWLock()
{
    pthread_rwlock_init(&rwlock_, nullptr);
}

NativeRWLock::~NativeRWLock()
{
    pthread_rwlock_destroy(&rwlock_);
}

bool NativeRWLock::ReadLock()
{
    return 0 == pthread_rwlock_rdlock(&rwlock_);
}

bool NativeRWLock::TryReadLock()
{
    return 0 == pthread_rwlock_tryrdlock(&rwlock_);
}

bool NativeRWLock::WriteLock()
{
    return 0 == pthread_rwlock_wrlock(&rwlock_);
}

bool NativeRWLock::TryWriteLock()
{
    return 0 == pthread_rwlock_trywrlock(&rwlock_);
}

void NativeRWLock::Unlock()
{
    pthread_rwlock_unlock(&rwlock_);
}

}}  // namespace fasmio::fiber_env

