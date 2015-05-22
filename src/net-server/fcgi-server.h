
#ifndef NET_SERVER_FCGI_SERVER_H_
#define NET_SERVER_FCGI_SERVER_H_

#include "./base-net-server.h"
#include "common/module-logger.h"
#include "interface/runtime-env.h"
#include "interface/logger.h"
#include "interface/net-server.h"
#include <memory>

namespace fasmio { namespace net_server {

class FcgiServer : public BaseNetServer
{
public:
    explicit FcgiServer(IRuntimeEnv*, ILogger*);

public:
    virtual INetServerConnHandler* CreateHandler(common::ModuleLogger *mlogger, runtime_env::ITCPSocket *sock, const char* addr);
};

}}  // namespace fasmio::net_server

#endif  // NET_SERVER_FCGI_SERVER_H_

