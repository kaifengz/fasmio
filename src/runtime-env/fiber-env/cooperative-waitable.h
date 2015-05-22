
#ifndef RUNTIME_ENV_FIBER_ENV_COOPERATIVE_WAITABLE_H_
#define RUNTIME_ENV_FIBER_ENV_COOPERATIVE_WAITABLE_H_

#include "interface/runtime-env/abs-time.h"
#include "./native-mutex.h"
#include "./wait-mgr.h"

namespace fasmio { namespace fiber_env {

class FiberImpl;
class ThreadContext;

class CooperativeWaitable
{
public:
    CooperativeWaitable();
    virtual ~CooperativeWaitable();

private:
    CooperativeWaitable(const CooperativeWaitable&);  // unimplemented
    CooperativeWaitable& operator= (const CooperativeWaitable&);  // unimplemented

public:
    bool Wait(const char* fiber_state, void (*func)(void*), void* arg);
    bool TimedWait(const runtime_env::ABSTime &time, const char* fiber_state, void (*func)(void*), void* arg);
    bool TimedWait(unsigned int timeout, const char* fiber_state, void (*func)(void*), void* arg);
    unsigned int Signal(unsigned int count);
    unsigned int Broadcast();

public:
    bool WaitingFiberTimedOut(WaitingFiberManager::WaitingFiber *fiber);

private:
    static void CompleteWait(ThreadContext*, void*);
    static void CompleteTimedWait(ThreadContext*, void*);

private:
    typedef WaitingFiberManager::WaitingFiber WaitingFiber;

    NativeMutex lock_;
    WaitingFiber waiting_list_;
    unsigned int waiting_count_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_COOPERATIVE_WAITABLE_H_

