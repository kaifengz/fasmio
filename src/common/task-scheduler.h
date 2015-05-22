
#ifndef COMMON_TASK_SCHEDULER_H_
#define COMMON_TASK_SCHEDULER_H_

#include "./thread-pool.h"
#include "./queue.h"

namespace fasmio { namespace common {

// TaskScheduler
class TaskScheduler : protected ThreadPool
{
public:
    explicit TaskScheduler(IRuntimeEnv* env, ILogger* logger, const char* thread_name);
    virtual ~TaskScheduler();

public:
    bool Start(unsigned long interval, void (*proc)(void*), void* arg);

    template <typename HostClass, void (HostClass::*Scheduler)()>
    bool Start(unsigned long interval, HostClass* host);

    void Stop();

protected:
    virtual void Worker();

private:
    unsigned int interval_;  // in milliseconds
    void (*proc_)(void*);
    void *arg_;
};

}}  // namespace fasmio::common

#include "./task-scheduler-aux.h"
#endif  // COMMON_TASK_SCHEDULER_H_

