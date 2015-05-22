
#ifndef RUNTIME_ENV_PTHREAD_ENV_PLATFORM_THREAD_H_
#define RUNTIME_ENV_PTHREAD_ENV_PLATFORM_THREAD_H_

#include <pthread.h>

namespace fasmio { namespace pthread_env {

class PlatformThread
{
public:
    PlatformThread(void (*func)(void*), void* arg, unsigned long stack_size = 0);
    ~PlatformThread();

private:
    PlatformThread(const PlatformThread&);  // unimplemented
    PlatformThread& operator= (const PlatformThread&);  // unimplemented

public:
    bool Start();
    bool Join();

    static void* GetSpecific();
    static bool  SetSpecific(void* arg);

private:
    static void* ThreadProc(void* arg);
    static void  InitKey();

private:
    void (* const func_)(void* arg);
    void* const arg_;
    const unsigned long stack_size_;
    pthread_t thread_;

    static pthread_key_t key_;
    static bool key_initialized_;
    static pthread_once_t key_once_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_PLATFORM_THREAD_H_

