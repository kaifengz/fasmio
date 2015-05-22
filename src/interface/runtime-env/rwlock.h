
#ifndef INTERFACE_RUNTIME_RWLOCK_H_
#define INTERFACE_RUNTIME_RWLOCK_H_

namespace fasmio { namespace runtime_env {

class IRWLock
{
public:
    virtual ~IRWLock() {}

public:
    virtual bool ReadLock() = 0;
    virtual bool TryReadLock() = 0;

    virtual bool WriteLock() = 0;
    virtual bool TryWriteLock() = 0;

    virtual void Unlock() = 0;
};

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_RWLOCK_H_

