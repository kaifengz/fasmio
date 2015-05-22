
#ifndef CONTAINER_QUEUE_IMPL_H_
#define CONTAINER_QUEUE_IMPL_H_

#include "common/queue.h"
#include "service/interface/task.h"
#include "service/interface/queue.h"
#include <string>

namespace fasmio { namespace container {

class QueueImpl : public service::IOutQueue
{
public:
    explicit QueueImpl(IRuntimeEnv* env, const char* name);
    virtual ~QueueImpl();

public:
    const char*              GetName    () const;
    bool                     ReAddTask  (service::ITask* task);

public:
    service::ITask*          GetTask    ();
    service::ITask*          GetTask    (bool blocking,
                                         unsigned timeout);
                             
    unsigned long            GetTasks   (service::ITask* tasks[],
                                         unsigned long count);
                             
    void                     TaskDone   ();
    void                     TaskDone   (unsigned long count);
                             
    unsigned long            GetDoingCount ();

    unsigned long            GetSize    ();
                             
    unsigned long            GetTraffic ();

public:
    virtual bool             AddTask    (service::ITask* task);

    virtual bool             SetFilter  (service::TaskFilter filter);

protected:
    static bool              FilterTask (service::ITask* task, void* arg);

protected:
    virtual void             OnAnteReAddTask  (service::ITask*);
    virtual void             OnAnteAddTask    (service::ITask*);
    virtual void             OnPostGetTask    (service::ITask*);
    virtual void             OnTaskDone       (unsigned long count);

private:
    typedef common::Queue<service::ITask> task_queue_t;
    task_queue_t task_queue_;
    std::string queue_name_;
    service::TaskFilter filter_;
};

}}  // namespace fasmio::container

namespace fasmio { namespace common {

template <typename QueueClass>
struct QueueTraits;

template <>
struct QueueTraits<fasmio::container::QueueImpl>
{
    typedef fasmio::container::QueueImpl queue_t;
    typedef fasmio::service::ITask       data_t;

    static data_t* Pop(queue_t *queue, bool blocking, unsigned int timeout)
    {
        return queue->GetTask(blocking, timeout);
    }

    static void TaskDone(queue_t *queue, unsigned int count)
    {
        queue->TaskDone(count);
    }
};

}}  // namespace fasmio::common

#endif  // CONTAINER_QUEUE_IMPL_H_

