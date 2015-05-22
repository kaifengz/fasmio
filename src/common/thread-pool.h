
#ifndef COMMON_THREAD_POOL_H_
#define COMMON_THREAD_POOL_H_

#include "interface/logger.h"
#include "interface/runtime-env.h"
#include "./module-logger.h"
#include <memory>
#include <string>
#include <vector>

namespace fasmio { namespace common {

class ThreadPool
{
public:
    explicit ThreadPool(IRuntimeEnv* env, ILogger* logger, const char* thread_name);
    virtual ~ThreadPool();

public:
    bool Start(unsigned int thread_count);
    void Stop();

protected:
    virtual void Worker() = 0;

protected:
    IRuntimeEnv* const env_;
    ILogger* const logger_;
    ModuleLogger mlogger_;
    std::unique_ptr<runtime_env::IEvent> const quit_event_;

private:
    static int Worker(void* arg);

private:
    std::string thread_pool_name_;

    typedef std::vector<runtime_env::IThread*> threads_t;
    threads_t threads_;
};

}}  // namespace fasmio::common

#endif  // COMMON_THREAD_POOL_H_

