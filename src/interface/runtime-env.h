
#ifndef INTERFACE_RUNTIME_ENV_H_
#define INTERFACE_RUNTIME_ENV_H_

#include "./runtime-env/abs-time.h"
#include "./runtime-env/condition.h"
#include "./runtime-env/event.h"
#include "./runtime-env/file.h"
#include "./runtime-env/mutex.h"
#include "./runtime-env/rwlock.h"
#include "./runtime-env/semaphore.h"
#include "./runtime-env/socket.h"
#include "./runtime-env/thread.h"

namespace fasmio {

class IRuntimeEnv
{
public:
    virtual ~IRuntimeEnv() {}

public:
    virtual runtime_env::ITCPSocket* NewTCPSocket() = 0;
    virtual runtime_env::IFile*      NewFile() = 0;
    virtual runtime_env::IMutex*     NewMutex(bool recursive) = 0;
    virtual runtime_env::IRWLock*    NewRWLock() = 0;
    virtual runtime_env::ICondition* NewCondition(runtime_env::IMutex *mutex) = 0;
    virtual runtime_env::IEvent*     NewEvent(bool init_state) = 0;
    virtual runtime_env::ISemaphore* NewSemaphore(unsigned int max_count, unsigned int init_count) = 0;

    virtual void Sleep(unsigned long milliseconds) = 0;
    virtual void Yield() = 0;
    virtual void Quit(int quit_code) = 0;

    virtual runtime_env::IThread* CreateThread(int (*func)(void*), void* arg, const char* name) = 0;
    virtual runtime_env::IThread* CurrentThread() = 0;

    virtual int RunMain(int (*main)(IRuntimeEnv*, int argc, char* argv[]), int argc, char* argv[]) = 0;
};

}  // namespace fasmio

#endif  // INTERFACE_RUNTIME_ENV_H_

