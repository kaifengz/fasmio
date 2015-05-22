
#include "./rwlock.h"

namespace fasmio { namespace pthread_env {

RWLock::RWLock()
{
    pthread_rwlock_init(&rwlock_, nullptr);
}

RWLock::~RWLock()
{
    pthread_rwlock_destroy(&rwlock_);
}

bool RWLock::ReadLock()
{
    return 0 == pthread_rwlock_rdlock(&rwlock_);
}

bool RWLock::TryReadLock()
{
    return 0 == pthread_rwlock_tryrdlock(&rwlock_);
}

bool RWLock::WriteLock()
{
    return 0 == pthread_rwlock_wrlock(&rwlock_);
}

bool RWLock::TryWriteLock()
{
    return 0 == pthread_rwlock_trywrlock(&rwlock_);
}

void RWLock::Unlock()
{
    pthread_rwlock_unlock(&rwlock_);
}

}}  // namespace fasmio::pthread_env

