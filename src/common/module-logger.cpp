
#include "./module-logger.h"

namespace fasmio { namespace common {

ModuleLogger::ModuleLogger(ILogger* logger, const char* module_name) :
    logger_(logger),
    module_name_(module_name)
{
}

#define LOG(level) \
    do { \
        va_list args; \
        va_start(args, format); \
        logger_->WriteLogV(module_name_, level, format, args); \
        va_end(args); \
    } while (0)

#define LOGV(level) \
    do { \
        logger_->WriteLogV(module_name_, level, format, args); \
    } while (0)

void ModuleLogger::Verbose(const char* format, ...)
{
    LOG(ILogger::VERBOSE);
}

void ModuleLogger::Info(const char* format, ...)
{
    LOG(ILogger::INFO);
}

void ModuleLogger::Error(const char* format, ...)
{
    LOG(ILogger::ERROR);
}

void ModuleLogger::VerboseV(const char* format, va_list args)
{
    LOGV(ILogger::VERBOSE);
}

void ModuleLogger::InfoV(const char* format, va_list args)
{
    LOGV(ILogger::INFO);
}

void ModuleLogger::ErrorV(const char* format, va_list args)
{
    LOGV(ILogger::ERROR);
}

}}  // namespace fasmio::common

