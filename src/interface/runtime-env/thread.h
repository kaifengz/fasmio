
#ifndef INTERFACE_RUNTIME_THREAD_H_
#define INTERFACE_RUNTIME_THREAD_H_

namespace fasmio { namespace runtime_env {

class IThread
{
protected:
    virtual ~IThread() {};

public:
    // To join the thread.  The IThread object is deleted upon Join returns.
    virtual int Join() = 0;

    // To get the name of the thread
    virtual const char* GetName() = 0;

    // To get the unique id of the thread
    virtual unsigned int GetID() = 0;

    // Set the thread as a daemon thread.  A daemon thread will be automatically
    // Join'ed when it quitted.  Note that the IThread object may be deleted
    // upon SetDaemon returns.
    //
    // Daemon thread are not joinable; try to join a daemon would result in
    // undefined behavior
    virtual void SetDaemon() = 0;

    virtual bool IsDaemon() = 0;
};

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_THREAD_H_

