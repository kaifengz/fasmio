
#ifndef RUNTIME_ENV_PTHREAD_ENV_EVENT_H_
#define RUNTIME_ENV_PTHREAD_ENV_EVENT_H_

#include "interface/runtime-env/event.h"
#include "./condition.h"
#include "./mutex.h"

namespace fasmio { namespace pthread_env {

class Event : public runtime_env::IEvent
{
public:
    explicit Event(bool init_state);
    virtual ~Event();

private:
    Event(const Event&);  // unimplemented
    Event& operator= (const Event&);  // unimplemented

public:
    virtual bool Wait();
    virtual bool TimedWait(const runtime_env::ABSTime &time);
    virtual bool TimedWait(unsigned int timeout);

    virtual bool Set();
    virtual bool Reset();
    virtual bool Pulse();
    virtual bool IsSet();

private:
    Mutex mutex_;
    Condition cond_;
    bool flag_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_EVENT_H_

