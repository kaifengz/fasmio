
#ifndef COMMON_AUTO_LOCK_H_
#define COMMON_AUTO_LOCK_H_

#include "interface/runtime-env.h"
#include <memory>

namespace fasmio { namespace common {

namespace detail {

template <typename LockClass, bool (LockClass::*LockFunc)()>
class GenericAutoLock
{
public:
    explicit GenericAutoLock(LockClass* lock) :
        lock_(lock)
    {
        locked_ = (lock_->*LockFunc)();
    }

    explicit GenericAutoLock(std::unique_ptr<LockClass> &lock_ptr) :
        lock_(lock_ptr.get())
    {
        locked_ = (lock_->*LockFunc)();
    }

    explicit GenericAutoLock(std::shared_ptr<LockClass> &lock_ptr) :
        lock_(lock_ptr.get())
    {
        locked_ = (lock_->*LockFunc)();
    }

    ~GenericAutoLock()
    {
        Unlock();
    }

public:
    operator bool ()
    {
        return locked_;
    }

    void Unlock()
    {
        if (locked_)
        {
            lock_->Unlock();
            locked_ = false;
        }
    }

private:
    LockClass * const lock_;
    bool locked_;
};

} // namespace detail

typedef detail::GenericAutoLock<runtime_env::IMutex, &runtime_env::IMutex::Lock> AutoLock;
typedef detail::GenericAutoLock<runtime_env::IRWLock, &runtime_env::IRWLock::ReadLock> AutoReadLock;
typedef detail::GenericAutoLock<runtime_env::IRWLock, &runtime_env::IRWLock::WriteLock> AutoWriteLock;

}}  // namespace fasmio::common

#endif  // COMMON_AUTO_LOCK_H_

