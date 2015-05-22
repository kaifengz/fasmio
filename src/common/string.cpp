
#include "./string.h"
#include <stdarg.h>

namespace fasmio { namespace common {

std::string format(const char* format, ...)
{
    char buff[4096];

    va_list args;
    va_start(args, format);
    vsnprintf(buff, sizeof(buff), format, args);
    va_end(args);

    return std::string(buff);
}

}}  // namespace fasmio::common

