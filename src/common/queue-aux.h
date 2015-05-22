
#ifndef COMMON_QUEUE_H_
#   error Do not include "queue-aux.h" directly, include "queue.h" instead
#endif

#ifndef COMMON_QUEUE_AUX_H_
#define COMMON_QUEUE_AUX_H_

namespace fasmio { namespace common {

template <typename Type>
Queue<Type>::Queue(IRuntimeEnv* env) :
    queue_(),
    unfinished_(0),
    lock_(env->NewMutex(false)),
    not_empty_(env->NewCondition(lock_.get())),
    all_task_done_(env->NewCondition(lock_.get()))
{
}

template <typename Type>
Queue<Type>::~Queue()
{
}

template <typename Type>
bool Queue<Type>::Push(Type *data)
{
    AutoLock autolock(lock_);

    queue_.push_back(data);
    not_empty_->Signal();

    return true;
}

template <typename Type>
bool Queue<Type>::RePush(Type *data)
{
    AutoLock autolock(lock_);

    queue_.push_back(data);
    --unfinished_;
    not_empty_->Signal();

    return true;
}

template <typename Type>
Type* Queue<Type>::Pop(bool blocking, unsigned int timeout)
{
    AutoLock autolock(lock_);
    Type *data = nullptr;

    if (queue_.empty() && blocking)
    {
        runtime_env::ABSTime deadline;
        deadline.Adjust(0, timeout*1000);
        while (true)
        {
            not_empty_->TimedWait(timeout);
            if (!queue_.empty())
                break;

            runtime_env::ABSTime now;
            if (now >= deadline)
                break;
        }
    }

    if (!queue_.empty())
    {
        ++unfinished_;
        data = queue_.front();
        queue_.pop_front();
    }

    return data;
}

template <typename Type>
void Queue<Type>::TaskDone(unsigned int count)
{
    AutoLock autolock(lock_);

    unfinished_ -= count;
    if (unfinished_ == 0 && queue_.empty())
        all_task_done_->Signal();
}

template <typename Type>
unsigned int Queue<Type>::UnfinishedCount()
{
    return unfinished_;
}

template <typename Type>
bool Queue<Type>::Join()
{
    AutoLock autolock(lock_);

    while (unfinished_ != 0 || !queue_.empty())
        all_task_done_->Wait();

    return true;
}

template <typename Type>
unsigned int Queue<Type>::Size() const
{
    AutoLock autolock(lock_);
    return queue_.size();
}

template <typename Type>
unsigned int Queue<Type>::Pick(
        Type* data[], unsigned int count,
        bool (*filter)(Type*, void* arg), void *arg)
{
    AutoLock autolock(lock_);

    unsigned picked = 0;
    for (typename std::list<Type*>::iterator iter = queue_.begin();
        iter != queue_.end(); )
    {
        if (!filter(*iter, arg))
        {
            ++iter;
            continue;
        }

        data[picked] = *iter;
        queue_.erase(iter++);
        if (++picked >= count)
            break;
    }

    unfinished_ += picked;
    return picked;
}

}}  // namespace fasmio::common

#endif  // COMMON_QUEUE_AUX_H_

