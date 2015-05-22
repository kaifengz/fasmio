
#ifndef RUNTIME_ENV_FIBER_ENV_NATIVE_CONDITION_H_
#define RUNTIME_ENV_FIBER_ENV_NATIVE_CONDITION_H_

#include <pthread.h>

#include "interface/runtime-env/condition.h"
#include "./native-mutex.h"

namespace fasmio { namespace fiber_env {

class NativeCondition : public runtime_env::ICondition
{
public:
    explicit NativeCondition(NativeMutex *mutex);
    virtual ~NativeCondition();

private:
    NativeCondition(const NativeCondition&);  // unimplemented
    NativeCondition& operator= (const NativeCondition&);  // unimplemented

public:
    virtual bool Acquire();
    virtual bool TryAcquire();
    virtual void Release();

    virtual bool Wait();
    virtual bool TimedWait(const runtime_env::ABSTime &time);
    virtual bool TimedWait(unsigned int timeout);

    virtual bool Signal();
    virtual bool Broadcast();

private:
    NativeMutex &mutex_;
    pthread_cond_t cond_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_NATIVE_CONDITION_H_

