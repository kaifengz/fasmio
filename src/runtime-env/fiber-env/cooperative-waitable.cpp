
#include "./cooperative-waitable.h"

#include <assert.h>

#include "runtime-env/interlocked.h"
#include "./fiber-env.h"
#include "./fiber-impl.h"

namespace fasmio { namespace fiber_env {

CooperativeWaitable::CooperativeWaitable() :
    lock_(),
    waiting_list_(),
    waiting_count_(0)
{
}

CooperativeWaitable::~CooperativeWaitable()
{
}

bool CooperativeWaitable::Wait(const char* fiber_state, void (*func)(void*), void* arg)
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    assert(thread_context != nullptr);
    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    WaitingFiber wfiber;
    wfiber.waitable_           = this;
    wfiber.fiber_              = fiber;
    wfiber.state_              = WaitingFiberManager::STATE_WAITING;
    wfiber.post_switch_action_ = func;
    wfiber.post_switch_arg_    = arg;

    lock_.Lock();
    waiting_list_.WListInsert(&wfiber);
    ++waiting_count_;
    thread_context->Schedule(CompleteWait, static_cast<void*>(&wfiber), fiber_state);

    assert(wfiber.reason_ == WaitingFiberManager::REASON_SIGNALED);
    return true;
}

void CooperativeWaitable::CompleteWait(ThreadContext*, void *arg)
{
    WaitingFiber *wfiber = reinterpret_cast<WaitingFiber*>(arg);
    assert(wfiber != nullptr);
    assert(wfiber->state_ == WaitingFiberManager::STATE_WAITING);

    if (wfiber->post_switch_action_)
        (wfiber->post_switch_action_)(wfiber->post_switch_arg_);
    wfiber->waitable_->lock_.Unlock();
}

bool CooperativeWaitable::TimedWait(const runtime_env::ABSTime &time, const char* fiber_state, void (*func)(void*), void* arg)
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    assert(thread_context != nullptr);
    FiberImpl *fiber = thread_context->GetCurrentFiber();
    assert(fiber != nullptr);

    WaitingFiber wfiber;
    wfiber.waitable_           = this;
    wfiber.fiber_              = fiber;
    wfiber.state_              = WaitingFiberManager::STATE_TIMEDWAIT_PREPARING;
    wfiber.timeout_            = time;
    wfiber.post_switch_action_ = func;
    wfiber.post_switch_arg_    = arg;

    lock_.Lock();
    waiting_list_.WListInsert(&wfiber);
    ++waiting_count_;
    thread_context->Schedule(CompleteTimedWait, static_cast<void*>(&wfiber), fiber_state);

    const bool wait_succeed = (wfiber.reason_ == WaitingFiberManager::REASON_SIGNALED);
    return wait_succeed;
}

bool CooperativeWaitable::TimedWait(unsigned int timeout, const char* fiber_state, void (*func)(void*), void* arg)
{
    runtime_env::ABSTime time;
    time.Adjust(0, timeout * 1000);
    return TimedWait(time, fiber_state, func, arg);
}

void CooperativeWaitable::CompleteTimedWait(ThreadContext *thread_context, void *arg)
{
    WaitingFiber *wfiber = reinterpret_cast<WaitingFiber*>(arg);
    assert(wfiber != nullptr);
    assert(wfiber->state_ == WaitingFiberManager::STATE_TIMEDWAIT_PREPARING);

    if (wfiber->post_switch_action_)
        (wfiber->post_switch_action_)(wfiber->post_switch_arg_);
    wfiber->waitable_->lock_.Unlock();

    WaitingFiberManager *manager = thread_context->GetWaitingFiberManager();
    assert(manager != nullptr);
    manager->AddWaitingFiber(wfiber);

    wfiber->state_ = WaitingFiberManager::STATE_TIMEDWAIT_WAITING;
}

unsigned int CooperativeWaitable::Signal(unsigned int count)
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    assert(thread_context != nullptr);

    if (count == 0)
        return 0;

    // remove at most `count' waiting fibers from the wlist

    WaitingFiber waked_list;
    unsigned int waked_count = 0;

    lock_.Lock();
    WaitingFiber *wfiber = waiting_list_.wnext_;
    while (wfiber != &waiting_list_)
    {
        if (wfiber->state_ == WaitingFiberManager::STATE_WAITING ||
            runtime_env::interlocked::CompareExchange(&wfiber->state_,
                    WaitingFiberManager::STATE_TIMEDWAIT_SIGNALED,
                    WaitingFiberManager::STATE_TIMEDWAIT_WAITING))
        {
            WaitingFiber *waked = wfiber;
            wfiber = wfiber->wnext_;
            waiting_list_.WListRemove(waked);
            waked_list.WListInsert(waked);

            --waiting_count_;
            if (++waked_count >= count)
                break;
        }
        else
        {
            wfiber = wfiber->wnext_;
        }
    }
    lock_.Unlock();

    // ask WaitingFiberManager to forget all of the waked
    // waiting fibers and ready the fibers

    WaitingFiberManager *manager = thread_context->GetWaitingFiberManager();
    assert(manager != nullptr);

    WaitingFiber *waked = waked_list.wnext_;
    while (waked != &waked_list)
    {
        WaitingFiber *next = waked->wnext_;
        waked_list.WListRemove(waked);
        if (waked->state_ == WaitingFiberManager::STATE_TIMEDWAIT_SIGNALED)
            manager->WaitingFiberSignaled(waked);
        waked->reason_ = WaitingFiberManager::REASON_SIGNALED;
        thread_context->ReadyFiber(waked->fiber_);
        waked = next;
    }

    return waked_count;
}

unsigned int CooperativeWaitable::Broadcast()
{
    return Signal(static_cast<unsigned int>(-1));
}

bool CooperativeWaitable::WaitingFiberTimedOut(WaitingFiberManager::WaitingFiber *wfiber)
{
    lock_.Lock();
    waiting_list_.WListRemove(wfiber);
    --waiting_count_;
    lock_.Unlock();
    return true;
}

}}  // namespace fasmio::fiber_env

