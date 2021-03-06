
#ifndef INTERFACE_RUNTIME_MUTEX_H_
#define INTERFACE_RUNTIME_MUTEX_H_

namespace fasmio { namespace runtime_env {

class IMutex
{
public:
    virtual ~IMutex() {}

public:
    virtual bool Lock() = 0;
    virtual bool TryLock() = 0;
    virtual void Unlock() = 0;
};

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_MUTEX_H_

