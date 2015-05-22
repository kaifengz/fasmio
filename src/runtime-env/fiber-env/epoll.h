
#ifndef RUNTIME_ENV_FIBER_ENV_EPOLL_H_
#define RUNTIME_ENV_FIBER_ENV_EPOLL_H_

#include <map>
#include <set>
#include <vector>

#include "./native-mutex.h"
#include "./platform-thread.h"
#include "interface/runtime-env/socket.h"

namespace fasmio { namespace fiber_env {

class FiberEnv;
class FiberImpl;
class ThreadContext;

class Epoller
{
    typedef runtime_env::ITCPSocket::Condition Condition;

public:
    Epoller();
    ~Epoller();

public:
    bool Start(FiberEnv *env);
    void Stop();

    // call only call from a fiber
    int Wait(int fd, Condition cond, Condition *cond_ret, const char* state);
    int TimedWait(int fd, Condition cond, Condition *cond_ret, const runtime_env::ABSTime &time, const char* state);
    int TimedWait(int fd, Condition cond, Condition *cond_ret, unsigned int timeout, const char* state);

    // make an fd to non-block
    static int MakeNonBlock(int fd);

private:
    int Wait(int fd, Condition cond, Condition *cond_ret, const runtime_env::ABSTime *timeout, const char* state);
    static void CompleteWait(ThreadContext*, void*);

    // epoll thread proc
    static void EpollThreadProc(void*);
    void EpollThreadProc();

    // buzz related
    bool SetupBuzz();
    void DestroyBuzz();
    bool RingBuzz();
    void ResetBuzz();

private:
    struct WaitingFiber;
    struct LessWaitingFibersByTimeout
    {
        bool operator() (WaitingFiber*, WaitingFiber*);
    };

    typedef std::multimap<int, WaitingFiber*> waiting_fibers_by_id_t;
    typedef std::multiset<WaitingFiber*, LessWaitingFibersByTimeout> waiting_fibers_by_time_t;
    typedef std::vector<WaitingFiber*> to_wait_fibers_t;

    struct WaitingFiber
    {
        int fd_;
        FiberImpl *fiber_;
        unsigned int events_;
        const runtime_env::ABSTime *timeout_;
        int error_;
        waiting_fibers_by_id_t::iterator iter_by_id_;
        waiting_fibers_by_time_t::iterator iter_by_time_;
    };

private:
    int epoll_fd_;
    int buzz_rd_;
    int buzz_wr_;
    bool quit_flag_;

    FiberEnv *env_;

    NativeMutex to_wait_fibers_lock_;
    to_wait_fibers_t to_wait_fibers_;

    waiting_fibers_by_id_t wfibers_by_id_;
    waiting_fibers_by_time_t wfibers_by_time_;

    PlatformThread epoll_thread_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_EPOLL_H_

