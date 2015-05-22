
#include "./fcgi-server.h"
#include "./fcgi-conn.h"

namespace fasmio { namespace net_server {

FcgiServer::FcgiServer(IRuntimeEnv* env, ILogger* logger) :
    BaseNetServer(env, logger, "FcgiServer", 9000)
{
}

INetServerConnHandler* FcgiServer::CreateHandler(common::ModuleLogger *mlogger, runtime_env::ITCPSocket *sock, const char* addr)
{
    return new FcgiConnHandler(this, &mlogger_, sock, addr);
}

}}  // namespace fasmio::net_server

