
#ifndef RUNTIME_ENV_PTHREAD_ENV_SOCKET_H_
#define RUNTIME_ENV_PTHREAD_ENV_SOCKET_H_

#include "interface/runtime-env/socket.h"
#include <sys/time.h>

namespace fasmio { namespace pthread_env {

class TCPSocket : public runtime_env::ITCPSocket
{
public:
    TCPSocket();
    explicit TCPSocket(int sock);
    virtual ~TCPSocket();

public:
    virtual bool Bind(const char *addr, unsigned short port, bool reuse_addr);
    virtual bool Listen(int backlog);
    virtual ITCPSocket* Accept(char *addr, int addr_len);
    virtual bool Connect(const char *addr, unsigned short port);
    virtual long Send(const void* buff, unsigned long size);
    virtual long Receive(void* buff, unsigned long size);
    virtual long SendAll(const void* buff, unsigned long size);
    virtual long ReceiveAll(void* buff, unsigned long size);
    virtual Condition Wait(Condition cond);
    virtual Condition TimedWait(Condition cond, const runtime_env::ABSTime &time);
    virtual Condition TimedWait(Condition cond, unsigned int timeout);
    virtual bool Shutdown(ShutdownDirection);
    virtual void Close();
    virtual int GetLastError();

private:
    Condition Wait(Condition cond, struct timeval *tv);

private:
    int sock_;
    int error_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_SOCKET_H_

