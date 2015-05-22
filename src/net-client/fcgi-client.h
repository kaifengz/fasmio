
#ifndef NET_CLIENT_FCGI_CLIENT_H_
#define NET_CLIENT_FCGI_CLIENT_H_

#include "interface/logger.h"
#include "interface/net-client.h"
#include "interface/runtime-env.h"

namespace fasmio { namespace net_client {

class FcgiClient : public INetClient
{
public:
    explicit FcgiClient(IRuntimeEnv*, ILogger*);

public:
    virtual INetClientConnection* NewConnection();

private:
    IRuntimeEnv* const env_;
    ILogger* const logger_;
};

}}  // namespace fasmio::net_client

#endif  // NET_CLIENT_FCGI_CLIENT_H_

