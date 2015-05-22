
#ifndef COMMON_TASK_SCHEDULER_H_
#   error Do not include "task-scheduler-aux.h" directly, include "task-scheduler.h" instead
#endif

#ifndef COMMON_TASK_SCHEDULER_AUX_H_
#define COMMON_TASK_SCHEDULER_AUX_H_

namespace fasmio { namespace common {

namespace detail {

    template < typename HostClass,
               void (HostClass::*Scheduler)()>
    struct StaticMaker_TaskScheduler
    {
        static void Adaptor(void *arg)
        {
            HostClass *host = reinterpret_cast<HostClass*>(arg);
            (host->*Scheduler)();
        }
    };

}  // namespace detail

template <typename HostClass, void (HostClass::*Scheduler)()>
bool TaskScheduler::Start(unsigned long interval, HostClass* host)
{
    if (host == nullptr)
        return false;

    return Start(interval, detail::StaticMaker_TaskScheduler<HostClass, Scheduler>::Adaptor, static_cast<void*>(host));
}


}}  // namespace fasmio::common

#endif  // COMMON_TASK_SCHEDULER_AUX_H_

