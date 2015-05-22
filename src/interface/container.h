
#ifndef INTERFACE_CONTAINER_H_
#define INTERFACE_CONTAINER_H_

#include "./config.h"
#include "./net-client.h"
#include "./net-request.h"
#include "./stream.h"

namespace fasmio {

enum ResponseCode
{
    RC_OK = 0,

    RC_BAD_REQUEST = 400,
    RC_SERVICE_NOT_FOUND,
    RC_RESOURCE_NOT_FOUND,

    RC_SERVICE_ERROR = 500,
    RC_SERVICE_BUSY,
};

class IContainer
{
public:
    virtual ~IContainer() {}

public:
    virtual bool Initialize(INetClient*, IConfig*) = 0;
    virtual void Finalize() = 0;

    virtual ResponseCode PrepareRequest(INetRequest*) = 0;
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*) = 0;
};

}  // namespace fasmio

#endif  // INTERFACE_CONTAINER_H_

