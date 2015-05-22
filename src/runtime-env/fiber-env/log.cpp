
#include "./log.h"

#include <stdarg.h>
#include <stdio.h>

#ifdef ENABLE_LOG
#   include <sys/syscall.h>
#   include <sys/types.h>
#   include <unistd.h>
#   define gettid() (int)syscall(__NR_gettid)
#endif

namespace fasmio { namespace fiber_env {

#ifdef ENABLE_LOG

void log(const char* fmt, ...)
{
    char buff[1024];
    int len = 0;

    len += snprintf(buff + len, sizeof(buff) - len, "%d\t", gettid());

    va_list args;
    va_start(args, fmt);
    len += vsnprintf(buff + len, sizeof(buff) - len, fmt, args);
    va_end(args);

    len += snprintf(buff + len, sizeof(buff) - len, "\n");

    printf("%s", buff);
}
#endif

}}  // namespace fasmio::fiber_env

