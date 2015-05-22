
#ifndef INTERFACE_RUNTIME_SOCKET_H_
#define INTERFACE_RUNTIME_SOCKET_H_

namespace fasmio { namespace runtime_env {

class ABSTime;

class ITCPSocket
{
public:
    enum Condition
    {
        C_NONE     = 0x00,
        C_READABLE = 0x01,
        C_WRITABLE = 0x02,
        C_ANY      = 0x03,
        C_ALL      = 0x03,
    };

    enum ShutdownDirection
    {
        SHUT_RECV = 0,
        SHUT_SEND = 1,
        SHUT_BOTH = 2,
    };

public:
    virtual ~ITCPSocket() {};

public:
    virtual bool Bind(const char *addr, unsigned short port, bool reuse_addr) = 0;

    virtual bool Listen(int backlog) = 0;

    virtual ITCPSocket* Accept(char *addr, int addr_len) = 0;

    virtual bool Connect(const char *addr, unsigned short port) = 0;

    virtual long Send(const void* buff, unsigned long size) = 0;

    virtual long Receive(void* buff, unsigned long size) = 0;

    virtual long SendAll(const void* buff, unsigned long size) = 0;

    virtual long ReceiveAll(void* buff, unsigned long size) = 0;

    virtual Condition Wait(Condition cond) = 0;

    virtual Condition TimedWait(Condition cond, const ABSTime &time) = 0;

    virtual Condition TimedWait(Condition cond, unsigned int timeout) = 0;

    virtual bool Shutdown(ShutdownDirection) = 0;

    virtual void Close() = 0;

    virtual int GetLastError() = 0;
};

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_SOCKET_H_

