
#ifndef INTERFACE_NET_REQUEST_H_
#define INTERFACE_NET_REQUEST_H_

#include "./stream.h"

namespace fasmio {

class INetRequest
{
public:
    virtual ~INetRequest() {}

public:
    virtual const char* GetURL() = 0;
    virtual const char* GetMethod() = 0;
    virtual IIStream* GetContent() = 0;
};

}  // namespace fasmio

#endif  // INTERFACE_NET_REQUEST_H_

