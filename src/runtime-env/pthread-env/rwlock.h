
#ifndef RUNTIME_ENV_PTHREAD_ENV_RWLOCK_H_
#define RUNTIME_ENV_PTHREAD_ENV_RWLOCK_H_

#include <pthread.h>
#include "interface/runtime-env/rwlock.h"

namespace fasmio { namespace pthread_env {

class RWLock : public runtime_env::IRWLock
{
public:
    RWLock();
    virtual ~RWLock();

private:
    RWLock(const RWLock&);  // unimplemented
    RWLock& operator= (const RWLock&);  // unimplemented

public:
    virtual bool ReadLock();
    virtual bool TryReadLock();

    virtual bool WriteLock();
    virtual bool TryWriteLock();

    virtual void Unlock();

private:
    pthread_rwlock_t rwlock_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_RWLOCK_H_

