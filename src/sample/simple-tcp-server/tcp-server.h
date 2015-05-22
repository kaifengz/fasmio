
#ifndef SIMPLE_TCP_SERVER_H
#define SIMPLE_TCP_SERVER_H

#include "interface/runtime-env.h"

class ConnectionHandler
{
public:
    virtual ~ConnectionHandler() {}

public:
    virtual void HandleConnection(fasmio::runtime_env::ITCPSocket*, const char* addr) = 0;
};

class SimpleTCPServer
{
public:
    explicit SimpleTCPServer(fasmio::IRuntimeEnv* env);
    virtual ~SimpleTCPServer();

public:
    bool Start(unsigned short port, ConnectionHandler* (*conn_creator)(fasmio::IRuntimeEnv*));
    void Stop();

protected:
    void log(const char* format, ...);

private:
    static int listen_thread(void*);
    int listen_thread();

    static int connection_thread(void*);
    int connection_thread(fasmio::runtime_env::ITCPSocket*, const char* addr);

private:
    fasmio::IRuntimeEnv *env_;
    bool quit_flag_;
    unsigned short port_;
    ConnectionHandler* (*conn_creator_)(fasmio::IRuntimeEnv*);
};

#endif  // SIMPLE_TCP_SERVER_H

