
#include "./fiber-impl.h"

#include <assert.h>

#include "./fiber-env.h"
#include "runtime-env/interlocked.h"

namespace fasmio { namespace fiber_env {

static const char* const g_ready   = "ready";
static const char* const g_running = "running";

unsigned long FiberImpl::next_id_ = 0;

FiberImpl::FiberImpl(FiberEnv* env, int (*func)(void*), void* arg, unsigned long stack_size, const char* name) :
    env_(env),
    id_(runtime_env::interlocked::Increment(reinterpret_cast<long*>(&next_id_))),
    name_(name != nullptr ? name : ""),
    fiber_(new PlatformFiber(func, arg, stack_size)),
    state_(0),
    quit_code_(0),
    daemon_(false),
    running_count_(0),
    last_schedule_time_(0,0),
    fiber_status_(FS_INIT),
    running_time_(),
    ready_time_(),
    waiting_time_()
{
}

FiberImpl::~FiberImpl()
{
    // The PlatformFiber should be deleted, means, the fiber should be quitted
    // before trying to delete FiberImpl object
    assert(fiber_ == nullptr);
}

int FiberImpl::Join()
{
    return FiberEnv::JoinFiber_s(this);
}

const char* FiberImpl::GetName()
{
    return name_.c_str();
}

unsigned int FiberImpl::GetID()
{
    return id_;
}

void FiberImpl::SetDaemon()
{
    daemon_ = true;
    env_->RecycleFiberIfQuitted(this);
}

bool FiberImpl::IsDaemon()
{
    return daemon_;
}

void FiberImpl::SetState(const char* state)
{
    state_ = state;
}

void FiberImpl::SetStateFromRunning(const char* state)
{
    if (state_ == g_running)
        state_ = state;
}

void FiberImpl::SetStateToRunning()
{
    state_ = g_running;
}

void FiberImpl::SetStateToReady()
{
    state_ = g_ready;
}

void FiberImpl::SetQuitCode(int quit_code)
{
    quit_code_ = quit_code;
}

void FiberImpl::FreeResource()
{
    if (fiber_ != nullptr)
    {
        delete fiber_;
        fiber_ = nullptr;
    }
}

void FiberImpl::Dump(FILE* fp)
{
    fprintf(fp, "fiber #%ld, name = %s, state = %s\n",
        id_, name_.c_str(), state_);
    fprintf(fp, "\trunning-count = %ld, running-time = %ld.%06ld, waiting-time = %ld.%06ld, ready-time = %ld.%06ld\n",
        running_count_,
        running_time_.seconds(), running_time_.useconds(),
        waiting_time_.seconds(), waiting_time_.useconds(),
        ready_time_.seconds(), ready_time_.useconds());
}

void FiberImpl::ChangeStatus(FiberStatus status)
{
    assert(status >= 0);
    assert(status < FS_MAX);
    assert(status != FS_INIT);

    runtime_env::ABSTime now;

    switch (fiber_status_)
    {
    case FS_INIT:
    default:
        break;

    case FS_RUNNING:
        assert(last_schedule_time_.seconds() != 0);
        ++running_count_;
        running_time_ += now - last_schedule_time_;
        break;

    case FS_READY:
        assert(last_schedule_time_.seconds() != 0);
        ready_time_ += now - last_schedule_time_;
        break;

    case FS_WAITING:
        assert(last_schedule_time_.seconds() != 0);
        waiting_time_ += now - last_schedule_time_;
        break;
    }

    fiber_status_ = status;
    last_schedule_time_ = now;
}

}}  // namespace fasmio::fiber_env

