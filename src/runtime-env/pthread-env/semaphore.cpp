
#include "./semaphore.h"
#include <assert.h>

namespace fasmio { namespace pthread_env {

Semaphore::Semaphore(unsigned int max_count, unsigned int init_count) :
    mutex_(false),
    cond_(&mutex_),
    max_count_(max_count),
    count_(init_count)
{
    assert(max_count > 0);
    assert(max_count >= init_count);
}

Semaphore::~Semaphore()
{
}

bool Semaphore::Wait()
{
    mutex_.Lock();
    if (count_ == 0)
        cond_.Wait();
    assert(count_ > 0);
    --count_;
    mutex_.Unlock();
    return true;
}

bool Semaphore::TimedWait(const runtime_env::ABSTime &time)
{
    bool ret = true;
    mutex_.Lock();
    if (count_ == 0)
        ret = cond_.TimedWait(time);
    if (ret)
    {
        assert(count_ > 0);
        --count_;
    }
    mutex_.Unlock();
    return ret;
}

bool Semaphore::TimedWait(unsigned int timeout)
{
    bool ret = true;
    mutex_.Lock();
    if (count_ == 0)
        ret = cond_.TimedWait(timeout);
    if (ret)
    {
        assert(count_ > 0);
        --count_;
    }
    mutex_.Unlock();
    return ret;
}

bool Semaphore::Release()
{
    mutex_.Lock();
    assert(count_ >= 0);
    assert(count_ < max_count_);
    ++count_;
    cond_.Signal();
    mutex_.Unlock();
    return true;
}

}}  // namespace fasmio::pthread_env

