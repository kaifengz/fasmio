
#ifndef NET_CLIENT_CONN_BASE_H_
#define NET_CLIENT_CONN_BASE_H_

#include "interface/logger.h"
#include "interface/net-client.h"
#include "interface/runtime-env.h"
#include "common/module-logger.h"

#if 0
#   define PLOG         mlogger_.Verbose
#else
#   define PLOG         plog
    inline void plog(const char* format, ...) {}
#endif

namespace fasmio { namespace net_client {

class ConnBase : public INetClientConnection, protected IOStream, protected IIStream
{
public:
    explicit ConnBase(IRuntimeEnv*, ILogger*, const char* module_name);
    virtual ~ConnBase();

public:
    virtual bool Connect(const char* host_port);
    virtual bool Connect(const char* host, unsigned short port);

    virtual bool SendRequest(const char* method, const char* url);
    virtual bool SendRequest(const char* method, const char* url, const char* data);

    virtual IOStream* StartRequest(const char* method, const char* url);
    virtual IOStream* StartRequest(const char* method, const char* url, unsigned long content_length) = 0;
    virtual bool EndRequest(IOStream **is) = 0;

    virtual bool Retrieve();
    virtual bool Retrieve(int *status_code);
    virtual bool Retrieve(int *status_code, IIStream **is, unsigned long *content_length);

    virtual bool Close(IIStream **is);

protected:
    enum
    {
        kContentLengthNotSpecified = 0xFFFFFFFF,
    };

    virtual long Write(const void* buff, long size) = 0;
    virtual long Read(void* buff, long size) = 0;
    virtual unsigned short GetDefaultPort() = 0;
    virtual void OnError();

protected:
    IRuntimeEnv* const env_;
    ILogger* const logger_;
    common::ModuleLogger mlogger_;
    runtime_env::ITCPSocket *sock_;
};

}}  // namespace fasmio::net_client

#endif  // NET_CLIENT_CONN_BASE_H_

