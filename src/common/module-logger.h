
#ifndef COMMON_MODULE_LOGGER_H_
#define COMMON_MODULE_LOGGER_H_

#include "interface/logger.h"

namespace fasmio { namespace common {

class ModuleLogger
{
public:
    ModuleLogger(ILogger* logger, const char* module_name);

public:
    void Verbose (const char* format, ...);
    void Info    (const char* format, ...);
    void Error   (const char* format, ...);

    void VerboseV (const char* format, va_list args);
    void InfoV    (const char* format, va_list args);
    void ErrorV   (const char* format, va_list args);

private:
    ILogger* logger_;
    const char* module_name_;
};

}}  // namespace fasmio::common

#endif  // COMMON_MODULE_LOGGER_H_

