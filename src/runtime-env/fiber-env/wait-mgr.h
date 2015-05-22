
#ifndef RUNTIME_ENV_FIBER_ENV_WAIT_H_
#define RUNTIME_ENV_FIBER_ENV_WAIT_H_

#include <map>

#include "interface/runtime-env/abs-time.h"
#include "./native-condition.h"
#include "./native-mutex.h"
#include "./platform-thread.h"

namespace fasmio { namespace fiber_env {

class CooperativeWaitable;
class FiberImpl;
class FiberEnv;

class WaitingFiberManager
{
public:
    WaitingFiberManager();
    ~WaitingFiberManager();

public:
    bool Start(FiberEnv *env);
    void Stop();

public:
    enum WaitingFiberState
    {
        STATE_UNKNOWN = 0,
        STATE_SLEEPING,
        STATE_WAITING,
        STATE_TIMEDWAIT_PREPARING,
        STATE_TIMEDWAIT_WAITING,
        STATE_TIMEDWAIT_SIGNALED,
        STATE_TIMEDWAIT_TIMEDOUT,
    };

    enum StopWaitingReason
    {
        REASON_UNKNOWN = 0,
        REASON_SIGNALED,
        REASON_TIMEDOUT,
    };

    struct WaitingFiber;
    typedef std::multimap<runtime_env::ABSTime, WaitingFiber*> SleepingFibers;

    struct WaitingFiber
    {
    public:
        WaitingFiber();
        ~WaitingFiber();

        void WListInsert(WaitingFiber *fiber);
        void WListRemove(WaitingFiber *fiber);

    public:
        CooperativeWaitable *waitable_;
        FiberImpl *fiber_;
        long state_;
        runtime_env::ABSTime timeout_;
        void (*post_switch_action_)(void*);
        void *post_switch_arg_;
        StopWaitingReason reason_;

        WaitingFiber *wprev_, *wnext_;
        SleepingFibers::iterator self_;
        static SleepingFibers::iterator null_;
    };

public:
    bool AddWaitingFiber(WaitingFiber *fiber);
    bool WaitingFiberSignaled(WaitingFiber *fiber);

protected:
    static void WaitMgrThreadProc(void*);
    void WaitMgrThreadProc();

    unsigned long WakeTimedOutFibers(unsigned long* to_usleep);

private:
    NativeMutex lock_;
    SleepingFibers sleeping_fibers_;

    FiberEnv *env_;

    bool quit_flag_;
    PlatformThread wait_mgr_thread_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_WAIT_H_

