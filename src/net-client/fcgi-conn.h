
#ifndef NET_CLIENT_FCGI_CONN_H_
#define NET_CLIENT_FCGI_CONN_H_

#include "./conn-base.h"
#include "common/fastcgi.h"

namespace fasmio { namespace net_client {

class FcgiConn : public ConnBase
{
public:
    explicit FcgiConn(IRuntimeEnv*, ILogger*);
    virtual ~FcgiConn();

public:
    virtual IOStream* StartRequest(const char* method, const char* url, unsigned long content_length);
    virtual bool EndRequest(IOStream **is);
    virtual bool Close(IIStream **is);

protected:
    virtual long Write(const void* buff, long size);
    virtual long Read(void* buff, long size);
    virtual unsigned short GetDefaultPort();
    virtual void OnError();

private:
    bool SendBeginRequest();
    bool SendParams(const char* method, const char* url, unsigned long content_length);
    bool FlushStdin(const void* buff, long size, bool terminate_stdin);
    bool ReceiveEndRequest();

private:
    enum FcgiState
    {
        FS_INIT = 0,
        FS_BEGIN_REQUEST_SENT,
        FS_PARAMS_SENT,
        FS_STDIN_SENT,
        FS_STDIN_END,
        FS_STDOUT_RECEIVED,
        FS_STDOUT_END,
        FS_END_REQUEST_RECEIVED,
        FS_ERROR,
    };

private:
    FcgiState state_;
    unsigned char fcgi_rw_buff_[1024];
    unsigned long fcgi_stdin_write_end_;
    unsigned long fcgi_stdout_read_begin_;
    unsigned long fcgi_stdout_read_end_;
    unsigned long fcgi_stdout_to_read_;
    unsigned long fcgi_stdout_padding_;
};

}}  // namespace fasmio::net_client

#endif  // NET_CLIENT_FCGI_CONN_H_

