
#ifndef INTERFACE_LOGGER_H_
#define INTERFACE_LOGGER_H_

#include <stdarg.h>

namespace fasmio {

class ILogger
{
public:
    virtual ~ILogger() {}

public:
    enum LogLevel
    {
        VERBOSE = 0,
        INFO,
        ERROR,
    };

public:
    virtual void WriteLog(const char* module_name, LogLevel level, const char* format, ...) = 0;
    virtual void WriteLogV(const char* module_name, LogLevel level, const char* format, va_list args) = 0;
};

}  // namespace fasmio

#endif  // INTERFACE_LOGGER_H_

