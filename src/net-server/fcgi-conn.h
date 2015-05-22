
#ifndef NET_SERVER_FCGI_CONN_H_
#define NET_SERVER_FCGI_CONN_H_

#include "./base-net-server.h"
#include "common/fastcgi.h"
#include "common/module-logger.h"
#include "interface/runtime-env.h"
#include <string>
#include <vector>

namespace fasmio { namespace net_server {

class BaseNetServer;
class FcgiServer;
class FcgiRequest;

class FcgiConnHandler : public INetServerConnHandler, public INetRequest, public IIStream, public IOStream
{
    friend class FcgiRequest;

public:
    FcgiConnHandler(FcgiServer* server, common::ModuleLogger *mlogger, runtime_env::ITCPSocket *sock, const char* addr);
    virtual ~FcgiConnHandler();

public:
    // for INetServerConnHandler

    virtual void HandleConnection();

public:
    // for INetRequest

    virtual const char* GetURL();
    virtual const char* GetMethod();
    virtual IIStream* GetContent();

public:
    // for IIStream

    virtual long Read(void* buff, long size);

public:
    // for IOStream

    virtual long Write(const void* buff, long size);

private:
    enum ConnStatus
    {
        CS_CONN_INIT = 0,
        CS_ERROR,
        CS_BEGIN_REQUEST,
        CS_PARAMS,
        CS_PARAMS_END,
        CS_STDIN,
        CS_STDIN_END,
        CS_STDOUT,
        CS_END_REQUEST,
        CS_CONN_DONE,
    };

private:
    bool DoBeginRequest(const FCGI_Header &header);
    bool DoParams(const FCGI_Header &header);

    bool EatPadding(unsigned int padding_len);
    bool EatStdin();

    bool StartStdout();
    bool EndStdout();
    bool EndRequest(ResponseCode resp_code);

    bool ParseParams(const unsigned char* params, unsigned int params_len);
    bool PrepareRequest();
    bool PrepareRequestParam(const char* name, std::string &value);

    static const char* FcgiTypeToName(int type);

private:
    FcgiServer *server_;
    common::ModuleLogger &mlogger_;
    runtime_env::ITCPSocket *sock_;
    std::string addr_;

    ConnStatus status_;
    unsigned int curr_req_id_;
    unsigned int fcgi_flags_;

    unsigned char fcgi_sparams_[512];
    unsigned char *fcgi_dparams_;
    unsigned long fcgi_dparams_size_;
    unsigned char *fcgi_params_;
    struct ParamString
    {
        const unsigned char* pos_;
        unsigned int length_;
    };
    struct Param
    {
        ParamString name_;
        ParamString value_;
    };
    typedef std::vector<Param> Params;
    Params params_;

    std::string request_url_;
    std::string request_method_;

    unsigned char fcgi_rw_buff_[1024];
    unsigned long fcgi_stdin_read_begin_;
    unsigned long fcgi_stdin_read_end_;
    unsigned long fcgi_stdin_to_read_;
    unsigned long fcgi_stdin_padding_;
    unsigned long fcgi_stdout_write_end_;
};

}}  // namespace fasmio::net_server

#endif  // NET_SERVER_FCGI_CONN_H_
