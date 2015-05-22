
#ifndef RUNTIME_ENV_FIBER_ENV_QUEUE_H_
#define RUNTIME_ENV_FIBER_ENV_QUEUE_H_

#include <stdlib.h>
#include <deque>

#include "interface/runtime-env/abs-time.h"

namespace fasmio { namespace fiber_env {

template <typename Type, typename Mutex, typename Condition>
class Queue
{
public:
    Queue();
    ~Queue();

public:
    bool Push(Type *data);
    Type* Pop(bool blocking = true, unsigned int timeout = 0);
    unsigned int Size() const;

private:
    std::deque<Type*> queue_;
    mutable Mutex lock_;
    mutable Condition not_empty_;
};

template <typename Type, typename Mutex, typename Condition>
Queue<Type, Mutex, Condition>::Queue() :
    queue_(), lock_(), not_empty_(&lock_)
{
}

template <typename Type, typename Mutex, typename Condition>
Queue<Type, Mutex, Condition>::~Queue()
{
}

template <typename Type, typename Mutex, typename Condition>
bool Queue<Type, Mutex, Condition>::Push(Type *data)
{
    lock_.Lock();
    queue_.push_back(data);
    not_empty_.Signal();
    lock_.Unlock();

    return true;
}

template <typename Type, typename Mutex, typename Condition>
Type* Queue<Type, Mutex, Condition>::Pop(bool blocking, unsigned int timeout)
{
    Type *data = nullptr;

    lock_.Lock();
    if (queue_.empty() && blocking)
    {
        runtime_env::ABSTime deadline;
        deadline.Adjust(0, timeout*1000);
        while (true)
        {
            not_empty_.TimedWait(timeout);
            if (!queue_.empty())
                break;

            runtime_env::ABSTime now;
            if (now >= deadline)
                break;
        }
    }

    if (!queue_.empty())
    {
        data = queue_.front();
        queue_.pop_front();
    }
    lock_.Unlock();

    return data;
}

template <typename Type, typename Mutex, typename Condition>
unsigned int Queue<Type, Mutex, Condition>::Size() const
{
    lock_.Lock();
    unsigned int size = queue_.size();
    lock_.Unlock();
    return size;
}

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_QUEUE_H_

