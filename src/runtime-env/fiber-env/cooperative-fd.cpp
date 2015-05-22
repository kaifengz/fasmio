
#include "./cooperative-fd.h"

#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "./fiber-env.h"

namespace fasmio { namespace fiber_env {

CooperativeFD::CooperativeFD() :
    fd_(-1),
    epoller_(nullptr)
{
    ThreadContext *thread_context = FiberEnv::GetThreadContext();
    if (thread_context != nullptr)
        epoller_ = thread_context->GetEpoller();
}

CooperativeFD::~CooperativeFD()
{
    assert(fd_ < 0);
}

int CooperativeFD::Attach(int fd)
{
    int rc;

    if (fd_ >= 0)
        return -1;
    if (0 != (rc = Epoller::MakeNonBlock(fd)))
        return rc;
    assert(fd >= 0);

    fd_ = fd;
    return 0;
}

void CooperativeFD::Dettach()
{
    fd_ = -1;
}

long CooperativeFD::Write(const void* buff, unsigned long size, const char* state)
{
    assert(epoller_ != nullptr && fd_ >= 0);

    if (buff == nullptr)
        return 0;
    if (size == 0)
        return 0;

    int ret = write(fd_, reinterpret_cast<const char*>(buff), size);
    if (ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        if (0 != epoller_->Wait(fd_, runtime_env::ITCPSocket::C_WRITABLE, nullptr, state))
            return 0;
        ret = write(fd_, reinterpret_cast<const char*>(buff), size);
    }

    return ret;
}

long CooperativeFD::Read(void* buff, unsigned long size, const char* state)
{
    assert(epoller_ != nullptr && fd_ >= 0);

    if (buff == nullptr)
        return 0;
    if (size == 0)
        return 0;

    int ret = read(fd_, reinterpret_cast<char*>(buff), size);
    if (ret < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        if (0 != epoller_->Wait(fd_, runtime_env::ITCPSocket::C_READABLE, nullptr, state))
            return 0;
        ret = read(fd_, reinterpret_cast<char*>(buff), size);
    }

    return ret;
}

long CooperativeFD::WriteAll(const void* buff, unsigned long size, const char* state)
{
    assert(epoller_ != nullptr && fd_ >= 0);

    if (buff == nullptr)
        return 0;
    if (size == 0)
        return 0;

    unsigned long bytes = 0;
    while (bytes < size)
    {
        int ret = write(fd_, reinterpret_cast<const char*>(buff)+bytes, size-bytes);
        if (ret > 0)
            bytes += ret;
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            if (0 != epoller_->Wait(fd_, runtime_env::ITCPSocket::C_WRITABLE, nullptr, state))
                break;
        }
        else
            break;
    }

    if (bytes > 0)
        return bytes;
    return -1;
}

long CooperativeFD::ReadAll(void* buff, unsigned long size, const char* state)
{
    assert(epoller_ != nullptr && fd_ >= 0);

    if (buff == nullptr)
        return 0;
    if (size == 0)
        return 0;

    unsigned long bytes = 0;
    while (bytes < size)
    {
        int ret = read(fd_, reinterpret_cast<char*>(buff)+bytes, size-bytes);
        if (ret > 0)
            bytes += ret;
        else if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            if (0 != epoller_->Wait(fd_, runtime_env::ITCPSocket::C_READABLE, nullptr, state))
                break;
        }
        else
            break;
    }

    if (bytes > 0)
        return bytes;
    return -1;
}

int CooperativeFD::Wait(Condition cond, Condition *cond_ret, const char* state)
{
    assert(epoller_ != nullptr && fd_ >= 0);

    return epoller_->Wait(fd_, cond, cond_ret, state);
}

int CooperativeFD::TimedWait(Condition cond, Condition *cond_ret, const runtime_env::ABSTime &time, const char* state)
{
    assert(epoller_ != nullptr && fd_ >= 0);

    return epoller_->TimedWait(fd_, cond, cond_ret, time, state);
}

int CooperativeFD::TimedWait(Condition cond, Condition *cond_ret, unsigned int timeout, const char* state)
{
    assert(epoller_ != nullptr && fd_ >= 0);

    return epoller_->TimedWait(fd_, cond, cond_ret, timeout, state);
}

}}  // namespace fasmio::fiber_env

