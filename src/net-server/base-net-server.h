
#ifndef NET_SERVER_BASE_NET_SERVER_H_
#define NET_SERVER_BASE_NET_SERVER_H_

#include "common/module-logger.h"
#include "common/queue.h"
#include "common/task-processor.h"
#include "interface/logger.h"
#include "interface/net-server.h"
#include "interface/runtime-env.h"
#include <memory>

namespace fasmio { namespace net_server {

class INetServerConnHandler
{
public:
    virtual ~INetServerConnHandler() {}

public:
    virtual void HandleConnection() = 0;
};

class BaseNetServer : public INetServer
{
public:
    explicit BaseNetServer(IRuntimeEnv*, ILogger*, const char* module_name, unsigned short port);
    virtual ~BaseNetServer();

public:
    bool Start(IContainer*, IConfig*);
    void Stop();

protected:
    virtual INetServerConnHandler* CreateHandler(common::ModuleLogger *mlogger, runtime_env::ITCPSocket *sock, const char* addr) = 0;

private:
    static int listen_thread(void* arg);
    int listen_thread();

    void process_conn(INetServerConnHandler *);

    void CleanupUnhandledConn();

public:
    ResponseCode PrepareRequest(INetRequest*);
    ResponseCode ServeRequest(INetRequest*, IOStream*);

    const char* ResponseCodeToStatusLine(ResponseCode resp_code);

protected:
    IRuntimeEnv* const env_;
    ILogger* const logger_;
    const char* const module_name_;
    const unsigned short port_;

    common::ModuleLogger mlogger_;

private:
    IContainer* container_;

    bool quit_flag_;

    std::unique_ptr<runtime_env::ITCPSocket> listen_sock_;
    runtime_env::IThread *listen_thread_;

    typedef common::Queue<INetServerConnHandler> conn_queue_t;
    typedef common::TaskProcessor<BaseNetServer, INetServerConnHandler, conn_queue_t> conn_processor_t;
    conn_queue_t conn_queue_;
    conn_processor_t conn_processor_;
};

}}  // namespace fasmio::net_server

#endif  // NET_SERVER_BASE_NET_SERVER_H_

