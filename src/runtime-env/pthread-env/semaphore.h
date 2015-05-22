
#ifndef RUNTIME_ENV_PTHREAD_ENV_SEMAPHORE_H_
#define RUNTIME_ENV_PTHREAD_ENV_SEMAPHORE_H_

#include "interface/runtime-env/semaphore.h"
#include "./condition.h"
#include "./mutex.h"

namespace fasmio { namespace pthread_env {

class Semaphore : public runtime_env::ISemaphore
{
public:
    Semaphore(unsigned int max_count, unsigned int init_count);
    virtual ~Semaphore();

private:
    Semaphore(const Semaphore&);  // unimplemented
    Semaphore& operator= (const Semaphore&);  // unimplemented

public:
    virtual bool Wait();
    virtual bool TimedWait(const runtime_env::ABSTime &time);
    virtual bool TimedWait(unsigned int timeout);

    virtual bool Release();

private:
    Mutex mutex_;
    Condition cond_;
    unsigned int max_count_;
    unsigned int count_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_SEMAPHORE_H_

