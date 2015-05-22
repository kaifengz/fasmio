
#ifndef INTERFACE_RUNTIME_CONDITION_H_
#define INTERFACE_RUNTIME_CONDITION_H_

#include "./waitable.h"

namespace fasmio { namespace runtime_env {

class ABSTime;

class ICondition : public IWaitable
{
public:
    virtual ~ICondition() {}

public:
    virtual bool Acquire() = 0;
    virtual bool TryAcquire() = 0;
    virtual void Release() = 0;

    virtual bool Wait() = 0;
    virtual bool TimedWait(const ABSTime &time) = 0;
    virtual bool TimedWait(unsigned int timeout) = 0;

    virtual bool Signal() = 0;
    virtual bool Broadcast() = 0;
};

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_CONDITION_H_

