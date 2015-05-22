
#ifndef RUNTIME_ENV_FIBER_ENV_COOPERATIVE_EVENT_H_
#define RUNTIME_ENV_FIBER_ENV_COOPERATIVE_EVENT_H_

#include "interface/runtime-env/event.h"
#include "./cooperative-waitable.h"
#include "./native-mutex.h"

namespace fasmio { namespace fiber_env {

class CooperativeEvent : public runtime_env::IEvent
{
public:
    explicit CooperativeEvent(bool init_state = false);
    virtual ~CooperativeEvent();

private:
    CooperativeEvent(const CooperativeEvent&);  // unimplemented
    CooperativeEvent& operator= (const CooperativeEvent&);  // unimplemented

public:
    virtual bool Wait();
    virtual bool TimedWait(const runtime_env::ABSTime &time);
    virtual bool TimedWait(unsigned int timeout);

    virtual bool Set();
    virtual bool Reset();
    virtual bool Pulse();
    virtual bool IsSet();

private:
    static void CompleteWait(void*);

private:
    NativeMutex lock_;
    bool flag_;
    CooperativeWaitable waitable_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_COOPERATIVE_EVENT_H_

