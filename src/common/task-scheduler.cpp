
#include "./task-scheduler.h"

namespace fasmio { namespace common {

TaskScheduler::TaskScheduler(IRuntimeEnv* env, ILogger* logger, const char* thread_name) :
    ThreadPool(env, logger, thread_name),
    proc_(nullptr),
    arg_(nullptr)
{
}

TaskScheduler::~TaskScheduler()
{
}

bool TaskScheduler::Start(unsigned long interval, void (*proc)(void*), void* arg)
{
    interval_ = interval;
    if (nullptr == (proc_ = proc))
        return false;
    arg_ = arg;

    return ThreadPool::Start(1);
}

void TaskScheduler::Stop()
{
    ThreadPool::Stop();
}

void TaskScheduler::Worker()
{
    do
    {
        proc_(arg_);
    }
    while (!quit_event_->TimedWait(interval_));
}

}}  // namespace fasmio::common

