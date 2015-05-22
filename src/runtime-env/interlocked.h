
#ifndef RUNTIME_ENV_INTERLOCKED_H_
#define RUNTIME_ENV_INTERLOCKED_H_

namespace fasmio { namespace runtime_env { namespace interlocked {

    long Increment(long* addend);
    long Decrement(long* addend);

    bool CompareExchange(long* destination, long exchange, long comparand);

}}}  // namespace fasmio::runtime_env::interlocked

#endif  // RUNTIME_ENV_INTERLOCKED_H_

