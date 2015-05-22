
#ifndef RUNTIME_ENV_PTHREAD_ENV_THREAD_IMPL_H_
#define RUNTIME_ENV_PTHREAD_ENV_THREAD_IMPL_H_

#include <memory>
#include <string>

#include "interface/runtime-env/abs-time.h"
#include "interface/runtime-env/thread.h"
#include "./platform-thread.h"

namespace fasmio { namespace pthread_env {

class PthreadEnv;

class ThreadImpl : public runtime_env::IThread
{
    friend class PthreadEnv;

public:
    ThreadImpl(PthreadEnv* env, const char* name = nullptr);
    ThreadImpl(PthreadEnv* env, int (*func)(void*), void* arg, unsigned long stack_size = 0, const char* name = nullptr);
    virtual ~ThreadImpl();

private:
    ThreadImpl(const ThreadImpl&);  // unimplemented
    ThreadImpl& operator= (const ThreadImpl&);  // unimplemented

public:
    virtual int Join();
    virtual const char* GetName();
    virtual unsigned int GetID();
    virtual void SetDaemon();
    virtual bool IsDaemon();

public:
    bool Start();

    static void CurrentThread(ThreadImpl* thread);
    static ThreadImpl* CurrentThread();

private:
    static void ThreadEntry(void*);

private:
    enum ThreadType
    {
        TT_NORMAL = 0,
        TT_DAEMON,
        TT_MAIN_THREAD,
    };

    enum ThreadState
    {
        TS_NOT_START = 0,
        TS_STARTING,
        TS_RUNNING,
        TS_QUITTING,
    };

private:
    PthreadEnv * const env_;
    const std::string name_;
    PlatformThread *thread_;
    int (*func_)(void*);
    void *arg_;
    unsigned long id_;
    int quit_code_;
    ThreadType thread_type_;
    volatile ThreadState thread_state_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_THREAD_IMPL_H_

