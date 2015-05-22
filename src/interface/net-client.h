
#ifndef INTERFACE_NET_CLIENT_H_
#define INTERFACE_NET_CLIENT_H_

#include "./stream.h"

namespace fasmio {

class INetClientConnection
{
public:
    virtual ~INetClientConnection() {}

public:
    virtual bool Connect(const char* host_port) = 0;
    virtual bool Connect(const char* host, unsigned short port) = 0;

    virtual bool SendRequest(const char* method, const char* url) = 0;
    virtual bool SendRequest(const char* method, const char* url, const char* data) = 0;

    virtual IOStream* StartRequest(const char* method, const char* url) = 0;
    virtual IOStream* StartRequest(const char* method, const char* url, unsigned long content_length) = 0;
    virtual bool EndRequest(IOStream **is) = 0;

    virtual bool Retrieve() = 0;
    virtual bool Retrieve(int *status_code) = 0;
    virtual bool Retrieve(int *status_code, IIStream **is, unsigned long *content_length) = 0;

    virtual bool Close(IIStream **is) = 0;
};

class INetClient
{
public:
    virtual ~INetClient() {}

public:
    virtual INetClientConnection* NewConnection() = 0;
};

}  // namespace fasmio

#endif  // INTERFACE_NET_CLIENT_H_

