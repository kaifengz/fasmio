
#ifndef RUNTIME_ENV_FIBER_ENV_COOPERATIVE_FD_H_
#define RUNTIME_ENV_FIBER_ENV_COOPERATIVE_FD_H_

#include "./epoll.h"
#include "interface/runtime-env/socket.h"

namespace fasmio { namespace fiber_env {

class CooperativeFD
{
    typedef runtime_env::ITCPSocket::Condition Condition;

public:
    CooperativeFD();
    ~CooperativeFD();

public:
    int Attach(int fd);
    void Dettach();

    long Write(const void* buff, unsigned long size, const char* state);
    long Read(void* buff, unsigned long size, const char* state);
    long WriteAll(const void* buff, unsigned long size, const char* state);
    long ReadAll(void* buff, unsigned long size, const char* state);

    int Wait(Condition cond, Condition *cond_ret, const char* state);
    int TimedWait(Condition cond, Condition *cond_ret, const runtime_env::ABSTime &time, const char* state);
    int TimedWait(Condition cond, Condition *cond_ret, unsigned int timeout, const char* state);

private:
    int fd_;
    Epoller *epoller_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_COOPERATIVE_FD_H_

