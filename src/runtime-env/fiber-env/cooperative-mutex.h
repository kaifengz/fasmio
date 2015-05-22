
#ifndef RUNTIME_ENV_FIBER_ENV_COOPERATIVE_MUTEX_H_
#define RUNTIME_ENV_FIBER_ENV_COOPERATIVE_MUTEX_H_

#include <deque>

#include "interface/runtime-env/mutex.h"
#include "./native-mutex.h"

namespace fasmio { namespace fiber_env {

class FiberImpl;
class ThreadContext;

class CooperativeMutex : public runtime_env::IMutex
{
    friend class CooperativeCondition;

public:
    explicit CooperativeMutex(bool recursive = false);
    virtual ~CooperativeMutex();

private:
    CooperativeMutex(const CooperativeMutex&);  // unimplemented
    CooperativeMutex& operator= (const CooperativeMutex&);  // unimplemented

public:
    virtual bool Lock();
    virtual bool TryLock();
    virtual void Unlock();

private:
    static void CompleteLock(ThreadContext*, void*);

private:
    const bool recursive_;
    NativeMutex lock_lock_;
    unsigned int locked_;
    FiberImpl *owner_;
    std::deque<FiberImpl*> waiting_queue_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_COOPERATIVE_MUTEX_H_

