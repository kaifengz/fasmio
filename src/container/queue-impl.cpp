
#include "./queue-impl.h"

namespace fasmio { namespace container {

QueueImpl::QueueImpl(IRuntimeEnv* env, const char* name) :
    task_queue_(env),
    queue_name_(name),
    filter_(nullptr)
{
}

QueueImpl::~QueueImpl()
{
}

const char* QueueImpl::GetName() const
{
    return queue_name_.c_str();
}

bool QueueImpl::ReAddTask(service::ITask* task)
{
    OnAnteReAddTask(task);
    return task_queue_.RePush(task);
}

service::ITask* QueueImpl::GetTask()
{
    return GetTask(true, 0xFFFFFFFF);
}

service::ITask* QueueImpl::GetTask(bool blocking, unsigned timeout)
{
    service::ITask* task = task_queue_.Pop(blocking, timeout);
    if (task != nullptr)
        OnPostGetTask(task);
    return task;
}

unsigned long QueueImpl::GetTasks(service::ITask* tasks[], unsigned long count)
{
    return task_queue_.Pick(tasks, count, FilterTask, this);
}

void QueueImpl::TaskDone()
{
    TaskDone(1);
}

void QueueImpl::TaskDone(unsigned long count)
{
    OnTaskDone(count);
    task_queue_.TaskDone(count);
}

unsigned long QueueImpl::GetDoingCount()
{
    return task_queue_.UnfinishedCount();
}

bool QueueImpl::AddTask(service::ITask* task)
{
    if (task == nullptr)
        return false;

    OnAnteAddTask(task);
    task_queue_.Push(task);
    return true;
}

bool QueueImpl::SetFilter(service::TaskFilter filter)
{
    // TODO: QueueImpl::SetFilter
    return false;
}

unsigned long QueueImpl::GetSize()
{
    return task_queue_.Size();
}

unsigned long QueueImpl::GetTraffic()
{
    // TODO: QueueImpl::GetTraffic
    return 0;
}

bool QueueImpl::FilterTask (service::ITask* task, void* arg)
{
    QueueImpl *queue = reinterpret_cast<QueueImpl*>(arg);

    if (queue->filter_ == nullptr)
        return true;
    else
    {
        // TODO: QueueImpl::FilterTask
        return queue->filter_(nullptr /* service */, nullptr /* dest_host */, task);
    }
}

void QueueImpl::OnAnteAddTask(service::ITask*)
{
}

void QueueImpl::OnAnteReAddTask(service::ITask*)
{
}

void QueueImpl::OnPostGetTask(service::ITask*)
{
}

void QueueImpl::OnTaskDone(unsigned long count)
{
}

}}  // namespace fasmio::container

