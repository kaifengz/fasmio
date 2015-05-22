
#ifndef COMMON_LOGGER_H_
#define COMMON_LOGGER_H_

#include "interface/logger.h"
#include "interface/runtime-env.h"

namespace fasmio { namespace common {

class Logger : public ILogger
{
public:
    explicit Logger(IRuntimeEnv* env);

public:
    virtual void WriteLog(const char* module_name, LogLevel level, const char* format, ...);
    virtual void WriteLogV(const char* module_name, LogLevel level, const char* format, va_list args);

private:
    IRuntimeEnv* env_;
};

}}  // namespace fasmio::common

#endif  // COMMON_LOGGER_H_

