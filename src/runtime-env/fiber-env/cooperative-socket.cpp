
#include "cooperative-socket.h"

#include <arpa/inet.h>
#include <errno.h>
#include <memory.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

namespace fasmio { namespace fiber_env {

CooperativeTCPSocket::CooperativeTCPSocket() :
    sock_(-1),
    error_(0),
    fd_()
{
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0)
    {
        error_ = errno;
        return;
    }

    if (0 != (error_ = fd_.Attach(sock_)))
    {
        close(sock_);
        sock_ = -1;
        return;
    }
}

CooperativeTCPSocket::CooperativeTCPSocket(int sock) :
    sock_(sock),
    error_(0),
    fd_()
{
    if (0 != (error_ = Epoller::MakeNonBlock(sock_)))
    {
        close(sock_);
        sock_ = -1;
        return;
    }

    if (0 != (error_ = fd_.Attach(sock_)))
    {
        close(sock_);
        sock_ = -1;
        return;
    }
}

CooperativeTCPSocket::~CooperativeTCPSocket()
{
    Close();
}

bool CooperativeTCPSocket::Bind(const char *addr, unsigned short port, bool reuse_addr)
{
    if (addr == nullptr)
        return false;
    if (sock_ < 0)
        return false;

    if (reuse_addr)
    {
        int reuse_addr_flag = 1;
        if (0 != (error_ = setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR,
                                &reuse_addr_flag, sizeof(reuse_addr_flag))))
            return false;
    }

    sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    if (0 == inet_aton(addr, &saddr.sin_addr))
    {
        error_ = -1;
        return false;
    }
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);

    if (0 != bind(sock_, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr)))
    {
        error_ = errno;
        return false;
    }

    return true;
}

bool CooperativeTCPSocket::Listen(int backlog)
{
    if (sock_ < 0)
        return false;

    if (0 != listen(sock_, backlog))
    {
        error_ = errno;
        return false;
    }

    return true;
}

runtime_env::ITCPSocket* CooperativeTCPSocket::Accept(char *addr, int addr_len)
{
    if (sock_ < 0)
        return nullptr;

    sockaddr_in saddr;
    socklen_t saddr_len = sizeof(saddr);
    int sock = accept(sock_, reinterpret_cast<sockaddr*>(&saddr), &saddr_len);
    if (sock < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            if (0 != (error_ = fd_.Wait(runtime_env::ITCPSocket::C_READABLE, nullptr, "socket.accept")))
                return nullptr;

            sock = accept(sock_, reinterpret_cast<sockaddr*>(&saddr), &saddr_len);
        }

        if (sock < 0)
        {
            error_ = errno;
            return nullptr;
        }
    }

    if (addr != nullptr && addr_len > 0)
        inet_ntop(AF_INET, &(saddr.sin_addr), addr, addr_len);

    return new CooperativeTCPSocket(sock);
}

bool CooperativeTCPSocket::Connect(const char *addr, unsigned short port)
{
    if (sock_ < 0)
        return false;

    sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);

    {   // do DNS
        struct hostent hostent1, *hostentp = nullptr;
        char buff[1024];
        int error = 0;
        if (0 != gethostbyname_r(addr, &hostent1, buff, sizeof(buff), &hostentp, &error))
        {
            error_ = error;
            return false;
        }

        if (hostentp != nullptr &&
            hostentp->h_addr_list != nullptr &&
            hostentp->h_addr_list[0] != nullptr)
        {
            memcpy(&saddr.sin_addr, hostentp->h_addr_list[0], sizeof(saddr.sin_addr));
        }
        else
        {
            error_ = -1;
            return false;
        }
    }

    int ret = connect(sock_, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr));
    if (0 == ret)
        return true;
    if (EINPROGRESS != (error_ = errno))
        return false;

    if (0 != (error_ = fd_.Wait(runtime_env::ITCPSocket::C_WRITABLE, nullptr, "socket.connect")))
        return false;

    int connect_error = 0;
    socklen_t arg_len = sizeof(connect_error);
    if (0 == getsockopt(sock_, SOL_SOCKET, SO_ERROR, &connect_error, &arg_len))
        error_ = connect_error;
    else
        error_ = errno;

    return (error_ == 0);
}

long CooperativeTCPSocket::Send(const void* buff, unsigned long size)
{
    if (sock_ < 0)
        return -1;

    int ret = fd_.Write(buff, size, "socket.send");
    if (ret < 0)
        error_ = errno;
    return ret;
}

long CooperativeTCPSocket::Receive(void* buff, unsigned long size)
{
    if (sock_ < 0)
        return -1;

    int ret = fd_.Read(buff, size, "socket.recv");
    if (ret < 0)
        error_ = errno;
    return ret;
}

long CooperativeTCPSocket::SendAll(const void* buff, unsigned long size)
{
    if (sock_ < 0)
        return -1;

    int ret = fd_.WriteAll(buff, size, "socket.send");
    if (ret < 0)
        error_ = errno;
    return ret;
}

long CooperativeTCPSocket::ReceiveAll(void* buff, unsigned long size)
{
    if (sock_ < 0)
        return -1;

    int ret = fd_.ReadAll(buff, size, "socket.recv");
    if (ret < 0)
        error_ = errno;
    return ret;
}

CooperativeTCPSocket::Condition CooperativeTCPSocket::Wait(Condition cond)
{
    if (sock_ < 0)
        return C_NONE;

    if (0 != (error_ = fd_.Wait(cond, &cond, "socket.wait")))
        return C_NONE;

    return cond;
}

CooperativeTCPSocket::Condition CooperativeTCPSocket::TimedWait(Condition cond, const runtime_env::ABSTime &time)
{
    if (sock_ < 0)
        return C_NONE;

    if (0 != (error_ = fd_.TimedWait(cond, &cond, time, "socket.timedwait")))
        return C_NONE;

    return cond;
}

CooperativeTCPSocket::Condition CooperativeTCPSocket::TimedWait(Condition cond, unsigned int timeout)
{
    if (sock_ < 0)
        return C_NONE;

    if (0 != (error_ = fd_.TimedWait(cond, &cond, timeout, "socket.timedwait")))
        return C_NONE;

    return cond;
}

bool CooperativeTCPSocket::Shutdown(ShutdownDirection how)
{
    if (sock_ < 0)
        return -1;

    if (0 != shutdown(sock_, static_cast<int>(how)))
    {
        error_ = errno;
        return false;
    }

    return true;
}

void CooperativeTCPSocket::Close()
{
    if (sock_ >= 0)
    {
        fd_.Dettach();
        close(sock_);
        sock_ = -1;
    }
}

int CooperativeTCPSocket::GetLastError()
{
    return error_;
}

}}  // namespace fasmio::fiber_env

