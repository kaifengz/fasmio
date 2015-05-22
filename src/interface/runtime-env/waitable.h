
#ifndef INTERFACE_RUNTIME_WAITABLE_H_
#define INTERFACE_RUNTIME_WAITABLE_H_

namespace fasmio { namespace runtime_env {

class ABSTime;

class IWaitable
{
public:
    virtual ~IWaitable() {}

public:
    virtual bool Wait() = 0;
    virtual bool TimedWait(const ABSTime &time) = 0;
    virtual bool TimedWait(unsigned int timeout) = 0;
};

int WaitForMultipleObjects(int count, IWaitable* waitables[], bool wait_all, int timeout);

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_WAITABLE_H_

