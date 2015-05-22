
#ifndef RUNTIME_ENV_FIBER_ENV_COOPERATIVE_CONDITION_H_
#define RUNTIME_ENV_FIBER_ENV_COOPERATIVE_CONDITION_H_

#include <deque>

#include "interface/runtime-env/condition.h"
#include "./cooperative-mutex.h"
#include "./cooperative-waitable.h"

namespace fasmio { namespace fiber_env {

class CooperativeCondition : public runtime_env::ICondition
{
public:
    explicit CooperativeCondition(CooperativeMutex *mutex);
    virtual ~CooperativeCondition();

private:
    CooperativeCondition(const CooperativeCondition&);  // unimplemented
    CooperativeCondition& operator= (const CooperativeCondition&);  // unimplemented

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
    static void CompleteWait(void*);

private:
    CooperativeMutex &mutex_;
    CooperativeWaitable waitable_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_COOPERATIVE_CONDITION_H_

