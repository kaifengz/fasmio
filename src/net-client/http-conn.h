
#ifndef NET_CLIENT_HTTP_CONN_H_
#define NET_CLIENT_HTTP_CONN_H_

#include "./conn-base.h"

namespace fasmio { namespace net_client {

class HttpConn : public ConnBase
{
public:
    explicit HttpConn(IRuntimeEnv*, ILogger*);
    virtual ~HttpConn();

public:
    virtual IOStream* StartRequest(const char* method, const char* url, unsigned long content_length);
    virtual bool EndRequest(IOStream **is);

protected:
    virtual long Write(const void* buff, long size);
    virtual long Read(void* buff, long size);
    virtual unsigned short GetDefaultPort();
};

}}  // namespace fasmio::net_client

#endif  // NET_CLIENT_HTTP_CONN_H_

