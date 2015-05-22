
#include "./platform-fiber.h"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include "./fiber-env.h"

namespace fasmio { namespace fiber_env {

const unsigned long kDefaultStackSize = 1024 * 1024;

PlatformFiber::PlatformFiber() :
    need_recycle_(false), func_(nullptr), arg_(0)
{
    getcontext(&context_);
}

PlatformFiber::PlatformFiber(int (*func)(void*), void* arg, unsigned long stack_size) :
    need_recycle_(true), func_(func), arg_(arg)
{
    if (stack_size == 0)
        stack_size = kDefaultStackSize;

    char* fiber_stack = new char[stack_size];
    memset(fiber_stack, 0, stack_size);

    getcontext(&context_);
    context_.uc_stack.ss_sp = (void*)fiber_stack;
    context_.uc_stack.ss_size = stack_size;
    makecontext(&context_, (void (*)())FiberProc, 1, this);
}

PlatformFiber::~PlatformFiber()
{
    if (need_recycle_)
    {
        char* stack = reinterpret_cast<char*>(context_.uc_stack.ss_sp);
        delete []stack;
    }
}

bool PlatformFiber::SwitchTo(PlatformFiber *from)
{
    return 0 == swapcontext(&from->context_, &context_);
}

bool PlatformFiber::SwitchTo()
{
    return 0 == setcontext(&context_);
}

void PlatformFiber::FiberProc(void* arg)
{
    ThreadContext* thread_context = FiberEnv::GetThreadContext();
    assert(thread_context != nullptr);
    thread_context->InitFiberSchedule();

    int quit_code = -1;
    PlatformFiber* fiber = reinterpret_cast<PlatformFiber*>(arg);
    if (fiber != nullptr)
        quit_code = (fiber->func_)(fiber->arg_);
    FiberEnv::Quit_s(quit_code);
}

}}  // namespace fasmio::fiber_env

