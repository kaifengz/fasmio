
#ifndef RUNTIME_ENV_FIBER_ENV_NATIVE_MUTEX_H_
#define RUNTIME_ENV_FIBER_ENV_NATIVE_MUTEX_H_

#include <pthread.h>

#include "interface/runtime-env/mutex.h"

namespace fasmio { namespace fiber_env {

class NativeMutex : public runtime_env::IMutex
{
    friend class NativeCondition;

public:
    NativeMutex();
    virtual ~NativeMutex();

private:
    NativeMutex(const NativeMutex &);  // unimplemented
    NativeMutex& operator= (const NativeMutex &);  // unimplemented

public:
    virtual bool Lock();
    virtual bool TryLock();
    virtual void Unlock();

private:
    pthread_mutex_t mutex_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_NATIVE_MUTEX_H_

