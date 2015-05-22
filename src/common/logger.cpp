
#include "./logger.h"

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <algorithm>

namespace fasmio { namespace common {

Logger::Logger(IRuntimeEnv* env) :
    env_(env)
{
}

void Logger::WriteLog(const char* module_name, LogLevel level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteLogV(module_name, level, format, args);
    va_end(args);
}

void Logger::WriteLogV(const char* module_name, LogLevel level, const char* format, va_list args)
{
    static const char* level_names[] = {
        "VER", "INF", "ERR",
    };
    if (level < 0 || level > sizeof(level_names)/sizeof(level_names[0]))
        level = INFO;

    runtime_env::IThread *self = env_->CurrentThread();
    int thread_id = (self != nullptr ? self->GetID() : -1);
    const char* thread_name = (self != nullptr ? self->GetName() : "");

    struct timeval now;
    gettimeofday(&now, 0);

    struct tm tm_now;
    gmtime_r(&now.tv_sec, &tm_now);

    char buff[4096];
    const unsigned int head_len = snprintf(buff, sizeof(buff),
            "%04d-%02d-%02d %02d:%02d:%02d.%03d [%s,%02d,%s] %s: ",
            tm_now.tm_year+1900, tm_now.tm_mon+1, tm_now.tm_mday,
            tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec,
            (int)(now.tv_usec/1000),
            module_name, thread_id, thread_name, level_names[level]);
    const unsigned int msg_len = vsnprintf(
            buff+head_len, sizeof(buff)-head_len, format, args);
    const unsigned int cr_pos = std::min(head_len + msg_len, sizeof(buff)-2);
    buff[cr_pos] = '\n';
    buff[cr_pos+1] = '\0';
    fwrite(buff, 1, cr_pos+1, stdout);
}

}}  // namespace fasmio::common

