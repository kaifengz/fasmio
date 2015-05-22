
#ifndef COMMON_QUEUE_H_
#define COMMON_QUEUE_H_

#include <stdlib.h>
#include <list>

#include "interface/runtime-env.h"
#include "./auto-lock.h"

namespace fasmio { namespace common {

template <typename Type>
class Queue
{
public:
    typedef Type data_t;

public:
    explicit Queue(IRuntimeEnv* env);
    ~Queue();

public:
    bool Push(Type *data);
    bool RePush(Type *data);
    Type* Pop(bool blocking = true, unsigned int timeout = 0);
    void TaskDone(unsigned int count = 1);
    unsigned int UnfinishedCount();
    bool Join();
    unsigned int Size() const;

    unsigned int Pick(
            Type* data[], unsigned int count,
            bool (*filter)(Type*, void* arg), void *arg);

private:
    std::list<Type*> queue_;
    unsigned int unfinished_;
    mutable std::unique_ptr<runtime_env::IMutex> lock_;
    mutable std::unique_ptr<runtime_env::ICondition> not_empty_;
    mutable std::unique_ptr<runtime_env::ICondition> all_task_done_;
};

}}  // namespace fasmio::common

#include "./queue-aux.h"
#endif  // COMMON_QUEUE_H_

