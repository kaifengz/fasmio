
#ifndef COMMON_TASK_PROCESSOR_H_
#define COMMON_TASK_PROCESSOR_H_

#include "./thread-pool.h"

namespace fasmio { namespace common {

template <typename QueueClass>
struct QueueTraits
{
    typedef QueueClass queue_t;
    typedef typename QueueClass::data_t data_t;

    static data_t* Pop(QueueClass *queue, bool blocking, unsigned int timeout)
    {
        return queue->Pop(blocking, timeout);
    }

    static void TaskDone(QueueClass *queue, unsigned int count)
    {
        return queue->TaskDone(count);
    }
};


template <typename HostClass, typename TaskClass, typename QueueClass>
class TaskProcessor : protected ThreadPool
{
public:
    typedef HostClass  host_t;
    typedef TaskClass  task_t;
    typedef QueueClass queue_t;
    typedef void (*TaskHandler)(HostClass*, TaskClass* task);

public:
    explicit TaskProcessor(IRuntimeEnv* env, ILogger* logger, const char* thread_name);
    virtual ~TaskProcessor();

public:
    bool Start(queue_t &queue, unsigned int thread_count, HostClass* host, TaskHandler handler);

    template <void (HostClass::*handler)(TaskClass* task)>
    bool Start(queue_t &queue, unsigned int thread_count, HostClass* host);

    void Stop();

protected:
    virtual void Worker();

private:
    queue_t *queue_;
    host_t  *host_;
    TaskHandler handler_;
};

}}  // namespace fasmio::common

#include "./task-processor-aux.h"
#endif  // COMMON_TASK_PROCESSOR_H_

