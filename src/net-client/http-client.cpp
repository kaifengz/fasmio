
#include "./http-client.h"
#include "./http-conn.h"

namespace fasmio { namespace net_client {

HttpClient::HttpClient(IRuntimeEnv* env, ILogger* logger) :
    env_(env),
    logger_(logger)
{
}

INetClientConnection* HttpClient::NewConnection()
{
    return new HttpConn(env_, logger_);
}

}}  // namespace fasmio::net_client

