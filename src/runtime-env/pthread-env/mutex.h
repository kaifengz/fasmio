
#ifndef RUNTIME_ENV_PTHREAD_ENV_MUTEX_H_
#define RUNTIME_ENV_PTHREAD_ENV_MUTEX_H_

#include <pthread.h>
#include "interface/runtime-env/mutex.h"

namespace fasmio { namespace pthread_env {

class Mutex : public runtime_env::IMutex
{
    friend class Condition;

public:
    explicit Mutex(bool recursive);
    virtual ~Mutex();

private:
    Mutex(const Mutex &);  // unimplemented
    Mutex& operator= (const Mutex &);  // unimplemented

public:
    virtual bool Lock();
    virtual bool TryLock();
    virtual void Unlock();

private:
    pthread_mutex_t mutex_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_MUTEX_H_

