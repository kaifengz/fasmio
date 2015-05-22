
#ifndef RUNTIME_ENV_PTHREAD_ENV_PTHREAD_ENV_H_
#define RUNTIME_ENV_PTHREAD_ENV_PTHREAD_ENV_H_

#include "interface/runtime-env.h"
#include "./condition.h"
#include "./mutex.h"
#include <set>
#include <vector>

namespace fasmio { namespace pthread_env {

class ThreadImpl;

class PthreadEnv : public IRuntimeEnv
{
public:
    PthreadEnv();
    virtual ~PthreadEnv();

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

    virtual int RunMain(int (*main_proc)(IRuntimeEnv*, int, char*[]), int argc, char* argv[]);

public:
    void ThreadJoined(ThreadImpl *thread);
    bool AddDaemonThread(ThreadImpl *thread);

private:
    bool WaitAllThreads();
    unsigned int JoinDaemonThreads_nolock();

private:
    typedef std::set<ThreadImpl*> threads_t;

    threads_t threads_;
    threads_t daemon_threads_;
    Mutex threads_lock_;
    Condition threads_quit_cond_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_PTHREAD_ENV_H_

