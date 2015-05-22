
#ifndef RUNTIME_ENV_FIBER_ENV_NATIVE_RWLOCK_H_
#define RUNTIME_ENV_FIBER_ENV_NATIVE_RWLOCK_H_

#include <pthread.h>

#include "interface/runtime-env/rwlock.h"

namespace fasmio { namespace fiber_env {

class NativeRWLock : public runtime_env::IRWLock
{
public:
    NativeRWLock();
    virtual ~NativeRWLock();

private:
    NativeRWLock(const NativeRWLock&);  // unimplemented
    NativeRWLock& operator= (const NativeRWLock&);  // unimplemented

public:
    virtual bool ReadLock();
    virtual bool TryReadLock();

    virtual bool WriteLock();
    virtual bool TryWriteLock();

    virtual void Unlock();

private:
    pthread_rwlock_t rwlock_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_NATIVE_RWLOCK_H_

