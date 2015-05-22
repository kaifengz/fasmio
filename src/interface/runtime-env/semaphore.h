
#ifndef INTERFACE_RUNTIME_SEMAPHORE_H_
#define INTERFACE_RUNTIME_SEMAPHORE_H_

#include "./waitable.h"

namespace fasmio { namespace runtime_env {

class ABSTime;

class ISemaphore : public IWaitable
{
public:
    virtual ~ISemaphore() {}

public:
    virtual bool Wait() = 0;
    virtual bool TimedWait(const ABSTime &time) = 0;
    virtual bool TimedWait(unsigned int timeout) = 0;

    virtual bool Release() = 0;
};

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_SEMAPHORE_H_

