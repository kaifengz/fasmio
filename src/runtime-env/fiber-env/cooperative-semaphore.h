
#ifndef RUNTIME_ENV_FIBER_ENV_COOPERATIVE_SEMAPHORE_H_
#define RUNTIME_ENV_FIBER_ENV_COOPERATIVE_SEMAPHORE_H_

#include "interface/runtime-env/semaphore.h"
#include "./cooperative-waitable.h"
#include "./native-mutex.h"

namespace fasmio { namespace fiber_env {

class CooperativeSemaphore : public runtime_env::ISemaphore
{
public:
    CooperativeSemaphore(unsigned int max_count, unsigned int init_count);
    virtual ~CooperativeSemaphore();

private:
    CooperativeSemaphore(const CooperativeSemaphore&);  // unimplemented
    CooperativeSemaphore& operator= (const CooperativeSemaphore&);  // unimplemented

public:
    virtual bool Wait();
    virtual bool TimedWait(const runtime_env::ABSTime &time);
    virtual bool TimedWait(unsigned int timeout);

    virtual bool Release();

private:
    static void CompleteWait(void*);

private:
    NativeMutex lock_;
    unsigned int max_count_;
    unsigned int count_;
    CooperativeWaitable waitable_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_COOPERATIVE_SEMAPHORE_H_

