
#include "./fcgi-client.h"
#include "./fcgi-conn.h"

namespace fasmio { namespace net_client {

FcgiClient::FcgiClient(IRuntimeEnv* env, ILogger* logger) :
    env_(env),
    logger_(logger)
{
}

INetClientConnection* FcgiClient::NewConnection()
{
    return new FcgiConn(env_, logger_);
}

}}  // namespace fasmio::net_client

