
#ifndef RUNTIME_ENV_PTHREAD_ENV_CONDITION_H_
#define RUNTIME_ENV_PTHREAD_ENV_CONDITION_H_

#include <pthread.h>
#include "interface/runtime-env/condition.h"
#include "./mutex.h"

namespace fasmio { namespace pthread_env {

class Condition : public runtime_env::ICondition
{
public:
    explicit Condition(Mutex *mutex);
    virtual ~Condition();

private:
    Condition(const Condition&);  // unimplemented
    Condition& operator= (const Condition&);  // unimplemented

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
    Mutex &mutex_;
    pthread_cond_t cond_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_CONDITION_H_

