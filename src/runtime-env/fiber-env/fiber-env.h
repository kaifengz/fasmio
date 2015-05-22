
#ifndef RUNTIME_ENV_FIBER_ENV_FIBER_ENV_IMPL_H_
#define RUNTIME_ENV_FIBER_ENV_FIBER_ENV_IMPL_H_

#include <map>
#include <string>
#include <vector>

#include "interface/runtime-env.h"
#include "./epoll.h"
#include "./fiber-impl.h"
#include "./native-condition.h"
#include "./native-mutex.h"
#include "./platform-thread.h"
#include "./queue.h"
#include "./wait-mgr.h"

namespace fasmio { namespace fiber_env {

class ThreadContext;

class FiberEnv : public IRuntimeEnv
{
    friend class ThreadContext;

public:
    FiberEnv();
    virtual ~FiberEnv();

public:
    virtual runtime_env::ITCPSocket* NewTCPSocket();
    virtual runtime_env::IFile*      NewFile();
    virtual runtime_env::IMutex*     NewMutex(bool recursive);
    virtual runtime_env::IRWLock*    NewRWLock();
    virtual runtime_env::ICondition* NewCondition(runtime_env::IMutex *mutex);
    virtual runtime_env::IEvent*     NewEvent(bool init_state);
    virtual runtime_env::ISemaphore* NewSemaphore(unsigned int max_count, unsigned int init_count);

    virtual void Sleep(unsigned long milliseconds);
    virtual void Yield();
    virtual void Quit(int quit_code);

    virtual runtime_env::IThread* CreateThread(int (*func)(void*), void* arg, const char* name);
    virtual runtime_env::IThread* CurrentThread();

    virtual int RunMain(int (*fiber_main)(IRuntimeEnv*, int, char*[]), int argc, char* argv[]);

public:
    bool Start(unsigned int threads);
    void Stop();

    bool FiberizeThisThread();
    bool Join();

public:
    // diagnosis methods

    void RunConsole();
    void DumpFiberStates();

public:
    static void Sleep_s(unsigned long milliseconds);
    static void Yield_s();
    static void Quit_s(int status);
    static void Stop_s();
    static runtime_env::IThread* CreateThread_s(int (*func)(void*), void* arg, const char* name);
    static runtime_env::IThread* CurrentThread_s();
    static int JoinFiber_s(FiberImpl* fiber);

    // `state' should be a literal string
    static void SetCurrentFiberState(const char* state);
    static void SetCurrentFiberStateFromRunning(const char* state);

    static ThreadContext* GetThreadContext();
    static FiberEnv*  GetEnv();

public:
    void ReadyFiber(FiberImpl* fiber);
    void RecycleFiberIfQuitted(FiberImpl* fiber);

private:
    static int FiberMainWrapper(void*);

    static void ThreadProc(void *arg);
    void ThreadProc();

    void InitFiberSchedule(ThreadContext *thread_context);
    bool Schedule(ThreadContext *thread_context, void (*func)(ThreadContext*, void*), void *arg, const char* state);

private:
    static void CompleteSleep     (ThreadContext*, void*);
    static void CompleteYield     (ThreadContext*, void*);
    static void CompleteQuit      (ThreadContext*, void*);
    static void CompleteJoinFiber (ThreadContext*, void*);

private:
    bool quit_flag_;

    typedef std::map<unsigned long, FiberImpl*> fibers_t;
    fibers_t live_fibers_;  // maps from fiber-id to fiber itself
    fibers_t quitted_fibers_;  // maps from fiber-id to fiber itself
    fibers_t joining_fibers_;  // maps from target fiber-id to the joining fiber
    NativeMutex fibers_lock_;
    NativeCondition no_live_fibers_cond_;

    typedef Queue<FiberImpl, NativeMutex, NativeCondition> queue_t;
    queue_t ready_queue_;

    std::vector<PlatformThread*> threads_;
    volatile long fiberized_threads_;

    WaitingFiberManager waiting_fiber_manager_;
    Epoller epoller_;
};


class ThreadContext
{
    friend class FiberEnv;

public:
    FiberImpl* GetCurrentFiber();
    bool Schedule(const char* state);
    bool Schedule(void (*func)(ThreadContext*, void*), void *arg, const char* state);
    void ReadyFiber(FiberImpl* fiber);

    void InitFiberSchedule();

    WaitingFiberManager* GetWaitingFiberManager();
    Epoller* GetEpoller();

private:
    FiberEnv *env_;
    PlatformFiber *schedule_fiber_;
    FiberImpl *current_fiber_;
    FiberImpl *next_fiber_;
    void (*post_switch_action_)(ThreadContext*, void*);
    void *post_switch_arg_;
};


}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_FIBER_ENV_IMPL_H_

