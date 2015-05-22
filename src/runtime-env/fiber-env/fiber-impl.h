
#ifndef RUNTIME_ENV_FIBER_ENV_FIBER_IMPL_H_
#define RUNTIME_ENV_FIBER_ENV_FIBER_IMPL_H_

#include <memory>
#include <string>

#include "interface/runtime-env/abs-time.h"
#include "interface/runtime-env/thread.h"
#include "./platform-fiber.h"

namespace fasmio { namespace fiber_env {

class FiberEnv;
class ThreadContext;

class FiberImpl : public runtime_env::IThread
{
    friend class FiberEnv;
    friend class ThreadContext;

public:
    FiberImpl(FiberEnv *env, int (*func)(void*), void* arg, unsigned long stack_size = 0, const char* name = nullptr);
    virtual ~FiberImpl();

private:
    FiberImpl(const FiberImpl&);  // unimplemented
    FiberImpl& operator= (const FiberImpl&);  // unimplemented

public:
    virtual int Join();
    virtual const char* GetName();
    virtual unsigned int GetID();
    virtual void SetDaemon();
    virtual bool IsDaemon();

public:
    void SetState(const char* state);
    void SetStateFromRunning(const char* state);
    void SetStateToRunning();
    void SetStateToReady();

    void SetQuitCode(int quit_code);
    void FreeResource();

    void Dump(FILE* fp);

public:
    enum FiberStatus
    {
        FS_INIT = 0,
        FS_RUNNING,
        FS_READY,
        FS_WAITING,

        FS_MAX,
    };

    void ChangeStatus(FiberStatus status);

private:
    static unsigned long next_id_;

    FiberEnv *const env_;
    const unsigned long id_;
    const std::string name_;
    PlatformFiber *fiber_;
    const char* state_;
    int quit_code_;
    bool daemon_;

    // performance data
    unsigned long running_count_;
    runtime_env::ABSTime last_schedule_time_;
    FiberStatus fiber_status_;
    runtime_env::TimeSpan running_time_;
    runtime_env::TimeSpan ready_time_;
    runtime_env::TimeSpan waiting_time_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_FIBER_IMPL_H_

