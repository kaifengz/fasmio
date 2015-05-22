
#ifndef INTERFACE_NET_SERVER_H_
#define INTERFACE_NET_SERVER_H_

#include "./config.h"
#include "./container.h"

namespace fasmio {

class INetServer
{
public:
    virtual ~INetServer() {}

public:
    virtual bool Start(IContainer*, IConfig*) = 0;
    virtual void Stop() = 0;
};

}  // namespace fasmio

#endif  // INTERFACE_NET_SERVER_H_

