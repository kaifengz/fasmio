
#include "./base-net-server.h"

namespace fasmio { namespace net_server {

const unsigned int kNetServerConnThreadCount = 10;

BaseNetServer::BaseNetServer(IRuntimeEnv* env, ILogger* logger, const char* module_name, unsigned short port) :
    env_(env),
    logger_(logger),
    module_name_(module_name),
    port_(port),
    mlogger_(logger, module_name),
    container_(nullptr),
    quit_flag_(false),
    listen_sock_(),
    listen_thread_(nullptr),
    conn_queue_(env),
    conn_processor_(env, logger, "netsvr-conn-thr")
{
}

BaseNetServer::~BaseNetServer()
{
}

bool BaseNetServer::Start(IContainer* container, IConfig* config)
{
    if (container == nullptr)
        return false;
    container_ = container;

    mlogger_.Info("Starting %s ...", module_name_);

    listen_sock_.reset(env_->NewTCPSocket());
    quit_flag_ = false;

    const char* listen_ip = "0.0.0.0";
    const unsigned short listen_port = port_;
    if (!listen_sock_->Bind(listen_ip, listen_port, true))
    {
        mlogger_.Error("Failed to bind %s:%d", listen_ip, listen_port);
        return false;
    }

    if (!listen_sock_->Listen(5))
        return false;
    mlogger_.Info("Listening at %s:%d", listen_ip, listen_port);

    if (nullptr == (listen_thread_ = env_->CreateThread(
                    listen_thread, this, "netsvr-listen-thr")))
        return false;

    if (!conn_processor_.Start<&BaseNetServer::process_conn>(
                    conn_queue_, kNetServerConnThreadCount, this))
        return false;
    mlogger_.Info("%d connection threads started", kNetServerConnThreadCount);

    mlogger_.Info("%s started", module_name_);
    return true;
}

void BaseNetServer::Stop()
{
    mlogger_.Info("Stopping %s ...", module_name_);

    quit_flag_ = true;
    listen_thread_->Join();
    listen_thread_ = nullptr;
    listen_sock_.reset();

    conn_processor_.Stop();

    CleanupUnhandledConn();

    container_ = nullptr;
    mlogger_.Info("%s stopped", module_name_);
}

int BaseNetServer::listen_thread(void* arg)
{
    BaseNetServer *server = reinterpret_cast<BaseNetServer*>(arg);
    return server->listen_thread();
}

int BaseNetServer::listen_thread()
{
    if (listen_sock_.get() == nullptr)
        return 1;

    while (!quit_flag_)
    {
        runtime_env::ITCPSocket::Condition cond =
            listen_sock_->TimedWait(runtime_env::ITCPSocket::C_READABLE, 10);
        if (0 == (cond & runtime_env::ITCPSocket::C_READABLE))
            continue;

        char addr[256];
        runtime_env::ITCPSocket *sock = listen_sock_->Accept(addr, sizeof(addr));
        if (sock == nullptr)
            break;

        mlogger_.Verbose("Incoming connection from %s", addr);
        INetServerConnHandler *handler = CreateHandler(&mlogger_, sock, addr);
        if (handler == nullptr)
            delete sock;
        else
        {
            // Now the handler object hold the socket and responsible
            // the deletion of it.
            // handler object will be deleted once the connection is finished.
            conn_queue_.Push(handler);
        }
    }

    return 0;
}

void BaseNetServer::process_conn(INetServerConnHandler *handler)
{
    if (nullptr != handler)
    {
        handler->HandleConnection();
        delete handler;
    }
}

void BaseNetServer::CleanupUnhandledConn()
{
    unsigned int unhandled = 0;
    while (true)
    {
        INetServerConnHandler *handler = conn_queue_.Pop(false);
        if (handler == nullptr)
            break;

        delete handler;
        conn_queue_.TaskDone();
        ++unhandled;
    }

    if (unhandled > 0)
        mlogger_.Info("%d unhandled connections dropped", unhandled);
}

ResponseCode BaseNetServer::PrepareRequest(INetRequest *request)
{
    return container_->PrepareRequest(request);
}

ResponseCode BaseNetServer::ServeRequest(INetRequest *request, IOStream *ostream)
{
    return container_->ServeRequest(request, ostream);
}

const char* BaseNetServer::ResponseCodeToStatusLine(ResponseCode resp_code)
{
    switch (resp_code)
    {
    case RC_OK:                   return nullptr;
    case RC_BAD_REQUEST:          return "400 Bad Request\r\n";
    case RC_SERVICE_NOT_FOUND:
    case RC_RESOURCE_NOT_FOUND:   return "404 Not Found\r\n";
    case RC_SERVICE_ERROR:        return "500 Internal Server Error\r\n";
    case RC_SERVICE_BUSY:         return "503 Server Busy\r\n";
    default:                      return "500 Internal Server Error\r\n";
    }
}

}}  // namespace fasmio::net_server

