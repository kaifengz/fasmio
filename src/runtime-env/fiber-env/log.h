
#ifndef RUNTIME_ENV_FIBER_ENV_LOG_H_
#define RUNTIME_ENV_FIBER_ENV_LOG_H_

namespace fasmio { namespace fiber_env {

#ifdef DEBUG
#   define ENABLE_LOG
#endif

#ifdef ENABLE_LOG
void log(const char* fmt, ...);
#else
static inline void log(const char* fmt, ...)
{
}
#endif

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_LOG_H_

