
#include "./service-base.h"

#define DEFAULT_MIN_THROUGHPUT          1000
#define DEFAULT_OVERLOAD_COEFF          1.0
#define DEFAULT_UNDERLOAD_COEFF         0.8

#define LOG(LevelV)  \
    do { \
        va_list args; \
        va_start(args, format); \
        mlogger_.LevelV(format, args); \
        va_end(args); \
    } while (0)

namespace fasmio { namespace service { namespace sdk {

ServiceBase::ServiceBase(const char* name, IServiceAgent *agent, IRuntimeEnv* env, ILogger *logger) :
    name_(name),
    agent_(agent),
    logger_(logger),
    mlogger_(logger, name),
    min_throughput_(DEFAULT_MIN_THROUGHPUT),
    overload_coeff_(DEFAULT_OVERLOAD_COEFF),
    underload_coeff_(DEFAULT_UNDERLOAD_COEFF),
    env_(env)
{
}

ServiceBase::~ServiceBase()
{
}

bool ServiceBase::Initialize()
{
    return OnInitialize();
}

void ServiceBase::Finalize()
{
    OnFinalize();
}

bool ServiceBase::RegisterPopSlot(const char* name, IOutQueue **queue)
{
    return agent_->RegisterPopSlot(name, queue);
}

bool ServiceBase::IsOverload()
{
    // TODO: ServiceBase::IsOverload
    return false;
}

bool ServiceBase::IsUnderload()
{
    // TODO: ServiceBase::IsUnderload
    return true;
}

void ServiceBase::Verbose(const char* format, ...)
{
    LOG(VerboseV);
}

void ServiceBase::Info(const char* format, ...)
{
    LOG(InfoV);
}

void ServiceBase::Error(const char* format, ...)
{
    LOG(ErrorV);
}

bool ServiceBase::SetFlowControlMinThroughput(unsigned long min_throughput)
{
    min_throughput_ = min_throughput;
    return true;
}

bool ServiceBase::SetFlowControlOverloadCoeff(double overload_coeff)
{
    overload_coeff_ = overload_coeff;
    return true;
}

bool ServiceBase::SetFlowControlUnderloadCoeff(double underload_coeff)
{
    underload_coeff_ = underload_coeff;
    return true;
}

}}}  // namespace fasmio::service::sdk

