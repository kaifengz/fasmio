
#include "tcp-server.h"

#include <stdarg.h>
#include <stdio.h>

#include <string>

struct incoming_conn
{
    SimpleTCPServer *server;
    fasmio::runtime_env::ITCPSocket *sock;
    std::string addr;
};

SimpleTCPServer::SimpleTCPServer(fasmio::IRuntimeEnv* env) :
    env_(env)
{
}

SimpleTCPServer::~SimpleTCPServer()
{
}

bool SimpleTCPServer::Start(unsigned short port, ConnectionHandler* (*conn_creator)(fasmio::IRuntimeEnv*))
{
    port_ = port;
    conn_creator_ = conn_creator;
    quit_flag_ = false;

    if (env_ == nullptr)
        return false;

    if (NULL == env_->CreateThread(listen_thread, this, "listen-thread"))
        return false;

    return true;
}

void SimpleTCPServer::Stop()
{
    quit_flag_ = true;
}

void SimpleTCPServer::log(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

int SimpleTCPServer::listen_thread(void* arg)
{
    SimpleTCPServer *httpd = reinterpret_cast<SimpleTCPServer*>(arg);
    return httpd->listen_thread();
}

int SimpleTCPServer::listen_thread()
{
    fasmio::runtime_env::ITCPSocket *socket = env_->NewTCPSocket();
    if (socket == NULL)
        return 1;

    if (!socket->Bind("0.0.0.0", port_, true))
        return 1;

    if (!socket->Listen(5))
        return 1;

    log("Listening at port %d", port_);

    while (!quit_flag_)
    {
        char addr[256];
        fasmio::runtime_env::ITCPSocket *sock = socket->Accept(addr, sizeof(addr));
        if (sock != NULL)
        {
            incoming_conn *conn = new incoming_conn;
            conn->server = this;
            conn->sock = sock;
            conn->addr = addr;
            if (conn == NULL)
                delete sock;
            else
            {
                fasmio::runtime_env::IThread *conn_thread = env_->CreateThread(connection_thread, conn, "conn-thread");
                if (conn_thread != NULL)
                    conn_thread->SetDaemon();
                else
                {
                    delete conn;
                    delete sock;
                }
            }
        }
    }

    return 0;
}

int SimpleTCPServer::connection_thread(void* arg)
{
    incoming_conn *conn = reinterpret_cast<incoming_conn*>(arg);
    SimpleTCPServer *server = conn->server;

    ConnectionHandler *handler = server->conn_creator_(server->env_);
    if (handler != NULL)
    {
        handler->HandleConnection(conn->sock, conn->addr.c_str());
        delete handler;
    }

    delete conn->sock;
    delete conn;
    return 0;
}

