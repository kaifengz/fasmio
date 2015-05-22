
#ifndef COMMON_TASK_PROCESSOR_H_
#   error Do not include "task-processor-aux.h" directly, include "task-processor.h" instead
#endif

#ifndef COMMON_TASK_PROCESSOR_AUX_H_
#define COMMON_TASK_PROCESSOR_AUX_H_

namespace fasmio { namespace common {

namespace detail {

    template < typename HostClass,
               typename TaskClass,
               void (HostClass::*handler)(TaskClass*)>
    struct StaticMaker_TaskProcessor
    {
        static void Adaptor(HostClass *host, TaskClass *task)
        {
            (host->*handler)(task);
        }
    };

}  // namespace detail


template <typename HostClass, typename TaskClass, typename QueueClass>
TaskProcessor<HostClass, TaskClass, QueueClass>::TaskProcessor(IRuntimeEnv* env, ILogger* logger, const char* thread_name) :
    ThreadPool(env, logger, thread_name),
    queue_(nullptr),
    host_(nullptr),
    handler_(nullptr)
{
}

template <typename HostClass, typename TaskClass, typename QueueClass>
TaskProcessor<HostClass, TaskClass, QueueClass>::~TaskProcessor()
{
}

template <typename HostClass, typename TaskClass, typename QueueClass>
bool TaskProcessor<HostClass, TaskClass, QueueClass>::Start(queue_t &queue, unsigned int thread_count, HostClass *host, TaskHandler handler)
{
    queue_ = &queue;
    if (nullptr == (host_ = host))
        return false;
    if (nullptr == (handler_ = handler))
        return false;

    return ThreadPool::Start(thread_count);
}

template <typename HostClass, typename TaskClass, typename QueueClass>
template <void (HostClass::*handler)(TaskClass* task)>
bool TaskProcessor<HostClass, TaskClass, QueueClass>::Start(queue_t &queue, unsigned int thread_count, HostClass* host)
{
    queue_ = &queue;
    handler_ = detail::StaticMaker_TaskProcessor<HostClass, TaskClass, handler>::Adaptor;
    if (nullptr == (host_ = host))
        return false;

    return ThreadPool::Start(thread_count);
}

template <typename HostClass, typename TaskClass, typename QueueClass>
void TaskProcessor<HostClass, TaskClass, QueueClass>::Stop()
{
    ThreadPool::Stop();
}

template <typename HostClass, typename TaskClass, typename QueueClass>
void TaskProcessor<HostClass, TaskClass, QueueClass>::Worker()
{
    while (!quit_event_->IsSet())
    {
        TaskClass *task = QueueTraits<queue_t>::Pop(queue_, true, 10);

        if (nullptr == task)
            continue;

        handler_(host_, task);
        QueueTraits<queue_t>::TaskDone(queue_, 1);
    }
}

}}  // namespace fasmio::common

#endif  // COMMON_TASK_PROCESSOR_AUX_H_

