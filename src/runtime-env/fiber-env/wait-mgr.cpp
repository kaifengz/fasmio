
#include "./wait-mgr.h"

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "runtime-env/interlocked.h"
#include "./cooperative-waitable.h"
#include "./fiber-env.h"

namespace fasmio { namespace fiber_env {

const unsigned long kWaitMgrThreadStackSize = 256*1024;
const long kWaitPollingInterval = 1*1000; // micro-seconds

WaitingFiberManager::SleepingFibers::iterator WaitingFiberManager::WaitingFiber::null_(nullptr);

WaitingFiberManager::WaitingFiber::WaitingFiber() :
    waitable_(nullptr),
    fiber_(nullptr),
    state_(STATE_UNKNOWN),
    timeout_(),
    post_switch_action_(nullptr),
    post_switch_arg_(nullptr),
    reason_(REASON_UNKNOWN),
    wprev_(this),
    wnext_(this),
    self_(nullptr)
{
}

WaitingFiberManager::WaitingFiber::~WaitingFiber()
{
    // make sure this node has been removed from both
    // the wlist and slist when deletion
    assert(wprev_ == this);
    assert(wnext_ == this);
    assert(self_ == null_);
}

void WaitingFiberManager::WaitingFiber::WListInsert(WaitingFiber *fiber)
{
    fiber->wprev_ = this->wprev_;
    fiber->wnext_ = this;
    this->wprev_->wnext_ = fiber;
    this->wprev_ = fiber;
}

void WaitingFiberManager::WaitingFiber::WListRemove(WaitingFiber *fiber)
{
    fiber->wprev_->wnext_ = fiber->wnext_;
    fiber->wnext_->wprev_ = fiber->wprev_;
    fiber->wprev_ = fiber;
    fiber->wnext_ = fiber;
}

WaitingFiberManager::WaitingFiberManager() :
    lock_(),
    sleeping_fibers_(),
    env_(nullptr),
    quit_flag_(false),
    wait_mgr_thread_(WaitMgrThreadProc, static_cast<void*>(this), kWaitMgrThreadStackSize)
{
}

WaitingFiberManager::~WaitingFiberManager()
{
    Stop();
}

bool WaitingFiberManager::Start(FiberEnv *env)
{
    env_ = env;
    quit_flag_ = false;
    wait_mgr_thread_.Start();
    return true;
}

void WaitingFiberManager::Stop()
{
    quit_flag_ = true;
    wait_mgr_thread_.Join();
}

bool WaitingFiberManager::AddWaitingFiber(WaitingFiber *fiber)
{
    lock_.Lock();
    fiber->self_ = sleeping_fibers_.insert(std::make_pair(fiber->timeout_, fiber));
    lock_.Unlock();
    return true;
}

bool WaitingFiberManager::WaitingFiberSignaled(WaitingFiber *fiber)
{
    lock_.Lock();
    sleeping_fibers_.erase(fiber->self_);
    fiber->self_ = WaitingFiber::null_;
    lock_.Unlock();
    return true;
}

void WaitingFiberManager::WaitMgrThreadProc(void* arg)
{
    WaitingFiberManager *mgr = reinterpret_cast<WaitingFiberManager*>(arg);
    if (mgr != nullptr)
        mgr->WaitMgrThreadProc();
}

void WaitingFiberManager::WaitMgrThreadProc()
{
    while (!quit_flag_)
    {
        unsigned long to_usleep;
        WakeTimedOutFibers(&to_usleep);
        if (quit_flag_)
            break;
        usleep(to_usleep);
    }
}

unsigned long WaitingFiberManager::WakeTimedOutFibers(unsigned long* to_usleep)
{
    std::vector<WaitingFiber*> to_wake;
    bool has_next = false;
    runtime_env::TimeSpan to_next;

    lock_.Lock();
    {
        runtime_env::ABSTime now;
        for (SleepingFibers::iterator iter = sleeping_fibers_.begin();
            iter != sleeping_fibers_.end(); )
        {
            const runtime_env::ABSTime &timeout = iter->first;
            if (timeout > now)
            {
                has_next = true;
                to_next = timeout - now;
                break;
            }

            WaitingFiber* fiber = iter->second;
            if (fiber->state_ == STATE_SLEEPING ||
                runtime_env::interlocked::CompareExchange(&fiber->state_,
                        STATE_TIMEDWAIT_TIMEDOUT,
                        STATE_TIMEDWAIT_WAITING))
            {
                sleeping_fibers_.erase(iter++);
                to_wake.push_back(fiber);
            }
            else
            {
                ++iter;
            }
        }
    }
    lock_.Unlock();

    for (std::vector<WaitingFiber*>::iterator iter = to_wake.begin();
        iter != to_wake.end(); ++iter)
    {
        WaitingFiber *fiber = *iter;
        if (fiber->state_ == STATE_TIMEDWAIT_TIMEDOUT)
            fiber->waitable_->WaitingFiberTimedOut(fiber);
        fiber->reason_ = WaitingFiberManager::REASON_TIMEDOUT;
        fiber->self_ = WaitingFiber::null_;
        env_->ReadyFiber(fiber->fiber_);
    }

    if (has_next)
        *to_usleep = std::max(kWaitPollingInterval, to_next.seconds()*1000000 + to_next.useconds());
    else
        *to_usleep = kWaitPollingInterval;

    return to_wake.size();
}

}}  // namespace fasmio::fiber_env

