
#ifndef RUNTIME_ENV_FIBER_ENV_COOPERATIVE_RWLOCK_H_
#define RUNTIME_ENV_FIBER_ENV_COOPERATIVE_RWLOCK_H_

#include <deque>

#include "interface/runtime-env/rwlock.h"
#include "./native-mutex.h"

namespace fasmio { namespace fiber_env {

class FiberImpl;
class ThreadContext;

class CooperativeRWLock : public runtime_env::IRWLock
{
public:
    CooperativeRWLock();
    virtual ~CooperativeRWLock();

private:
    CooperativeRWLock(const CooperativeRWLock&);  // unimplemented
    CooperativeRWLock& operator= (const CooperativeRWLock&);  // unimplemented

public:
    virtual bool ReadLock();
    virtual bool TryReadLock();

    virtual bool WriteLock();
    virtual bool TryWriteLock();

    virtual void Unlock();

private:
    static void CompleteReadLock(ThreadContext *, void*);
    static void CompleteWriteLock(ThreadContext *, void*);

private:
    NativeMutex rwlock_lock_;
    std::deque<FiberImpl*> waiting_readers_;
    std::deque<FiberImpl*> waiting_writers_;
    unsigned long readers_;
    FiberImpl* writer_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_COOPERATIVE_RWLOCK_H_

