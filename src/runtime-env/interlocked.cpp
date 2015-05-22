
#include "./interlocked.h"

namespace fasmio { namespace runtime_env { namespace interlocked {

long Increment(long* addend)
{
    return __sync_add_and_fetch(addend, 1);
}

long Decrement(long* addend)
{
    return __sync_sub_and_fetch(addend, 1);
}

bool CompareExchange(long* destination, long exchange, long comparand)
{
    return __sync_bool_compare_and_swap(destination, comparand, exchange);
}

}}}  // namespace fasmio::runtime_env::interlocked

