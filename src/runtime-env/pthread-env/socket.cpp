
#include "./socket.h"
#include "interface/runtime-env/abs-time.h"

#include <arpa/inet.h>
#include <errno.h>
#include <memory.h>
#include <netdb.h>
#include <unistd.h>

namespace fasmio { namespace pthread_env {

TCPSocket::TCPSocket() :
    sock_(-1),
    error_(0)
{
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0)
        error_ = errno;
}

TCPSocket::TCPSocket(int sock) :
    sock_(sock),
    error_(0)
{
}

TCPSocket::~TCPSocket()
{
    Close();
}

bool TCPSocket::Bind(const char *addr, unsigned short port, bool reuse_addr)
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

bool TCPSocket::Listen(int backlog)
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

runtime_env::ITCPSocket* TCPSocket::Accept(char *addr, int addr_len)
{
    if (sock_ < 0)
        return nullptr;

    while (true)
    {
        sockaddr_in saddr;
        socklen_t saddr_len = sizeof(saddr);
        int sock = accept(sock_, reinterpret_cast<sockaddr*>(&saddr), &saddr_len);
        if (sock < 0)
        {
            error_ = errno;
            if (error_ == EAGAIN || error_ == EWOULDBLOCK)
                continue;
            return nullptr;
        }

        if (addr != nullptr && addr_len > 0)
            inet_ntop(AF_INET, &(saddr.sin_addr), addr, addr_len);

        return new TCPSocket(sock);
    }
}

bool TCPSocket::Connect(const char *addr, unsigned short port)
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

    if (0 != connect(sock_, reinterpret_cast<sockaddr*>(&saddr), sizeof(saddr)))
    {
        error_ = errno;
        return false;
    }

    return true;
}

long TCPSocket::Send(const void* buff, unsigned long size)
{
    if (sock_ < 0)
        return -1;
    if (buff == nullptr || size == 0)
        return 0;

    long ret = send(sock_, buff, size, 0);
    if (ret < 0)
        error_ = errno;
    return ret;
}

long TCPSocket::Receive(void* buff, unsigned long size)
{
    if (sock_ < 0)
        return -1;
    if (buff == nullptr || size == 0)
        return 0;

    long ret = recv(sock_, buff, size, 0);
    if (ret < 0)
        error_ = errno;
    return ret;
}

long TCPSocket::SendAll(const void* buff, unsigned long size)
{
    if (sock_ < 0)
        return -1;
    if (buff == nullptr || size == 0)
        return 0;

    unsigned long bytes = 0;
    while (bytes < size)
    {
        int ret = send(sock_, reinterpret_cast<const char*>(buff)+bytes, size-bytes, 0);
        if (ret > 0)
            bytes += ret;
        else
            break;
    }

    if (bytes > 0)
        return bytes;

    error_ = errno;
    return -1;
}

long TCPSocket::ReceiveAll(void* buff, unsigned long size)
{
    if (sock_ < 0)
        return -1;
    if (buff == nullptr || size == 0)
        return 0;

    unsigned long bytes = 0;
    while (bytes < size)
    {
        int ret = recv(sock_, reinterpret_cast<char*>(buff)+bytes, size-bytes, 0);
        if (ret > 0)
            bytes += ret;
        else
            break;
    }

    if (bytes > 0)
        return bytes;

    error_ = errno;
    return -1;
}

TCPSocket::Condition TCPSocket::Wait(Condition cond)
{
    return Wait(cond, nullptr);
}

TCPSocket::Condition TCPSocket::TimedWait(Condition cond, const runtime_env::ABSTime &time)
{
    struct timeval tv;
    tv.tv_sec = time.seconds();
    tv.tv_usec = time.useconds();
    return Wait(cond, &tv);
}

TCPSocket::Condition TCPSocket::TimedWait(Condition cond, unsigned int timeout)
{
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = timeout % 1000 * 1000;
    return Wait(cond, &tv);
}

TCPSocket::Condition TCPSocket::Wait(Condition cond, struct timeval *tv)
{
    if (0 == (cond & C_ANY))
        return C_NONE;

    fd_set rfds, wfds;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    if (0 != (cond & C_READABLE))
        FD_SET(sock_, &rfds);
    if (0 != (cond & C_WRITABLE))
        FD_SET(sock_, &wfds);

    int ret = select(sock_+1, &rfds, &wfds, nullptr, tv);
    if (ret < 0)
    {
        error_ = errno;
        return C_NONE;
    }

    int cond_ret = 0;
    if (FD_ISSET(sock_, &rfds))
        cond_ret |= C_READABLE;
    if (FD_ISSET(sock_, &wfds))
        cond_ret |= C_WRITABLE;
    return static_cast<Condition>(cond_ret);
}

bool TCPSocket::Shutdown(ShutdownDirection how)
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

void TCPSocket::Close()
{
    if (sock_ >= 0)
    {
        close(sock_);
        sock_ = -1;
    }
}

int TCPSocket::GetLastError()
{
    return error_;
}

}}  // namespace fasmio::pthread_env

