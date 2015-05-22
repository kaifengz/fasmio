
#include "./thread-pool.h"
#include "./string.h"

namespace fasmio { namespace common {

ThreadPool::ThreadPool(IRuntimeEnv* env, ILogger* logger, const char* name) :
    env_(env),
    logger_(logger),
    mlogger_(logger, "ThreadPool"),
    quit_event_(env->NewEvent(false)),
    thread_pool_name_(),
    threads_()
{
    if (nullptr != name)
        thread_pool_name_ = name;
    else
        thread_pool_name_ = format("Pool #%08X", static_cast<void*>(this));
}

ThreadPool::~ThreadPool()
{
}

bool ThreadPool::Start(unsigned int thread_count)
{
    if (!threads_.empty())
        return false;
    if (thread_count == 0)
        return false;

    quit_event_->Reset();
    for (unsigned i=0; i<thread_count; ++i)
    {
        runtime_env::IThread* thread = env_->CreateThread(
                    Worker, static_cast<void*>(this), thread_pool_name_.c_str());
        if (nullptr == thread)
            return false;

        threads_.push_back(thread);
    }

    return true;
}

void ThreadPool::Stop()
{
    quit_event_->Set();
    for (threads_t::iterator iter = threads_.begin();
        iter != threads_.end(); ++iter)
    {
        runtime_env::IThread* thread = *iter;
        thread->Join();
    }
    threads_.clear();
}

int ThreadPool::Worker(void* arg)
{
    ThreadPool* pool = reinterpret_cast<ThreadPool*>(arg);
    if (pool != nullptr)
        pool->Worker();
    return 0;
}

}}  // namespace fasmio::common

