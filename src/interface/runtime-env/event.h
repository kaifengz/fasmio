
#ifndef INTERFACE_RUNTIME_EVENT_H_
#define INTERFACE_RUNTIME_EVENT_H_

#include "./waitable.h"

namespace fasmio { namespace runtime_env {

class ABSTime;

class IEvent : public IWaitable
{
public:
    virtual ~IEvent() {}

public:
    virtual bool Wait() = 0;
    virtual bool TimedWait(const ABSTime &time) = 0;
    virtual bool TimedWait(unsigned int timeout) = 0;

    virtual bool Set() = 0;
    virtual bool Reset() = 0;
    virtual bool Pulse() = 0;
    virtual bool IsSet() = 0;
};

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_EVENT_H_

