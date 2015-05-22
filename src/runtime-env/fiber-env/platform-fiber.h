
#ifndef RUNTIME_ENV_FIBER_ENV_PLATFORM_FIBER_H_
#define RUNTIME_ENV_FIBER_ENV_PLATFORM_FIBER_H_

#include <ucontext.h>

namespace fasmio { namespace fiber_env {

class PlatformFiber
{
public:
    PlatformFiber();
    PlatformFiber(int (*func)(void* arg), void* arg, unsigned long stack_size = 0);
    ~PlatformFiber();

private:
    PlatformFiber(const PlatformFiber&);  // unimplemented
    PlatformFiber& operator= (const PlatformFiber&);  // unimplemented

public:
    bool SwitchTo(PlatformFiber *from);
    bool SwitchTo();

private:
    static void FiberProc(void *arg);

private:
    const bool need_recycle_;
    ucontext_t context_;
    int (*const func_)(void *arg);
    void* const arg_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_PLATFORM_FIBER_H_

