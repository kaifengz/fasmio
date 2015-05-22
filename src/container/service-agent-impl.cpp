
#include "./service-agent-impl.h"
#include "slot/delegated-query-handler.h"
#include "slot/delegated-update-handler.h"
#include "slot/delete-slot-handler.h"
#include "slot/pop-slot-handler.h"
#include "slot/push-slot-handler.h"
#include "slot/query-slot-handler.h"
#include "slot/update-slot-handler.h"
#include "common/auto-lock.h"
#include <assert.h>
#include <string.h>

namespace fasmio { namespace container {

const unsigned int kMaintStatusInterval = 1000; // milliseconds
const unsigned int kStateTransmitDelayPeriod = 15; // seconds

static const char* const state_names[] =
{
    "BORNING",
    "WAITING_CONFIG",
    "CONFIGURING",
    "WORKING",
    "ABOUT_TO_PAUSE",
    "PAUSING",
    "ABOUT_TO_RETIRE",
    "RETIRING",
    "RETIRED",
    "DYING",
    "ERROR",
};

ServiceAgentImpl::ServiceAgentImpl(IRuntimeEnv* env, ILogger* logger, const char* name) :
    env_(env),
    logger_(logger),
    name_(name),
    mlogger_(logger, name),
    service_(nullptr),
    state_(service::BORNING),
    state_time_(0),
    state_lock_(env->NewMutex(false)),
    maint_thread_(nullptr),
    maint_thread_quit_event_(env->NewEvent(false)),
    slots_rwlock_(env->NewRWLock()),
    slot_handlers_(),
    in_queues_(),
    out_queues_(),
    processors_(),
    timers_()
{
}

ServiceAgentImpl::~ServiceAgentImpl()
{
}

bool ServiceAgentImpl::InitializeAsynchronously(service::IService *srv)
{
    if (srv == nullptr)
        return false;
    if (service_ != nullptr)
        return false;
    service_ = srv;

    maint_thread_ = env_->CreateThread(MaintThreadProc, this, "maint-thr");
    if (maint_thread_ == nullptr)
        return false;

    return true;
}

void ServiceAgentImpl::Finalize()
{
    if (maint_thread_ == nullptr)
        return;

    maint_thread_quit_event_->Set();
    maint_thread_->Join();
    maint_thread_ = nullptr;
}

ResponseCode ServiceAgentImpl::PrepareRequest(INetRequest* request)
{
    assert(request != nullptr);

    common::AutoReadLock autolock(slots_rwlock_);
    SlotHandler *handler = LookupSlotHandler_NoLock(request);
    if (handler == nullptr)
        return RC_RESOURCE_NOT_FOUND;

    return ServiceAccessibleForRequest(request, handler);
}

ResponseCode ServiceAgentImpl::ServeRequest(INetRequest *request, IOStream *ostream)
{
    assert(request != nullptr && ostream != nullptr);

    common::AutoReadLock autolock(slots_rwlock_);
    SlotHandler *handler = LookupSlotHandler_NoLock(request);
    if (handler == nullptr)
        return RC_RESOURCE_NOT_FOUND;

    return handler->ServeRequest(request, ostream);
}

void ServiceAgentImpl::Verbose(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    mlogger_.VerboseV(format, args);
    va_end(args);
}

void ServiceAgentImpl::Info(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    mlogger_.InfoV(format, args);
    va_end(args);
}

void ServiceAgentImpl::Error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    mlogger_.ErrorV(format, args);
    va_end(args);
}

service::IService* ServiceAgentImpl::GetService()
{
    return service_;
}

const char* ServiceAgentImpl::GetServiceName()
{
    return name_.c_str();
}

service::ServiceState ServiceAgentImpl::GetServiceState()
{
    return state_;
}

bool ServiceAgentImpl::RegisterPushSlot(const char* name, service::TaskAllocator alloc, service::PushSlotHandler handler, unsigned concurrency)
{
    if (name == nullptr || alloc == nullptr || handler == nullptr || concurrency == 0)
        return false;
    if (!CheckSlotNameValidity(name))
        return false;

    queue_ptr_t queue(new QueueImpl(env_, name));

    if (!RegisterSlot(name, "POST", "Push", new PushSlotHandler(this, name, alloc, queue)))
        return false;

    {   common::AutoWriteLock autolock(slots_rwlock_);

        in_queues_.push_back(queue);

        const std::string processor_name(name);
        task_processor_t &processor = processors_[processor_name];
        processor.processor.reset(new common::TaskProcessor<
                    service::IService, service::ITask, QueueImpl>(env_, logger_, name));
        processor.concurrency = concurrency;
        processor.queue = queue;
        processor.handler = handler;
    }

    return true;
}

bool ServiceAgentImpl::RegisterPopSlot(const char* name, service::IOutQueue **pqueue)
{
    if (name == nullptr || pqueue == nullptr)
        return false;
    if (!CheckSlotNameValidity(name))
        return false;

    queue_ptr_t queue(new QueueImpl(env_, name));

    if (!RegisterSlot(name, "PUT", "Pop", new PopSlotHandler(this, name, queue)))
        return false;

    {   common::AutoWriteLock autolock(slots_rwlock_);
        out_queues_.push_back(queue);
        *pqueue = static_cast<service::IOutQueue*>(queue.get());
    }

    return true;
}

bool ServiceAgentImpl::RegisterQuerySlot(const char* name, service::QuerySlotHandler handler)
{
    if (name == nullptr || handler == nullptr)
        return false;
    if (!CheckSlotNameValidity(name))
        return false;

    return RegisterSlot(name, "GET", "Query", new QuerySlotHandler(this, name, handler));
}

bool ServiceAgentImpl::RegisterUpdateSlot(const char* name, service::TaskAllocator alloc, service::UpdateSlotHandler handler)
{
    if (name == nullptr || alloc == nullptr || handler == nullptr)
        return false;
    if (!CheckSlotNameValidity(name))
        return false;

    return RegisterSlot(name, "POST", "Update", new UpdateSlotHandler(this, name, alloc, handler));
}

bool ServiceAgentImpl::RegisterDeleteSlot(const char* name, service::DeleteSlotHandler handler)
{
    if (name == nullptr || handler == nullptr)
        return false;
    if (!CheckSlotNameValidity(name))
        return false;

    return RegisterSlot(name, "DELETE", "Delete", new DeleteSlotHandler(this, name, handler));
}

bool ServiceAgentImpl::RegisterTimer(const char* name, unsigned long interval, service::TimerHandler handler)
{
    if (name == nullptr || handler == nullptr)
        return false;

    const std::string timer_name(name);
    common::AutoWriteLock autolock(slots_rwlock_);

    timers_t::const_iterator iter = timers_.find(timer_name);
    if (iter != timers_.end())
    {
        Error("Timer %s already registered", timer_name.c_str());
        return false;
    }

    timer_t &timer = timers_[timer_name];
    timer.scheduler.reset(new common::TaskScheduler(env_, logger_, timer_name.c_str()));
    timer.interval = interval;
    timer.handler = handler;
    Info("Timer %s registered", timer_name.c_str());
    return true;
}

bool ServiceAgentImpl::NeedConfiguration(const char* configuration)
{
    // TODO: ServiceAgentImpl::NeedConfiguration
    return true;
}

bool ServiceAgentImpl::ServiceInitialized()
{
    if (false /* TODO: does need configuration */)
        return TryTransmitServiceState(service::BORNING, service::WAITING_CONFIG, nullptr);
    else
        return TryTransmitServiceState(service::BORNING, service::WORKING, nullptr);
}

bool ServiceAgentImpl::ServiceConfiguring()
{
    return TryTransmitServiceState(service::WAITING_CONFIG, service::CONFIGURING, &service::IService::OnConfiguring);
}

bool ServiceAgentImpl::ServiceConfigured()
{
    return TryTransmitServiceState(service::CONFIGURING, service::WORKING, &service::IService::OnConfigured);
}

bool ServiceAgentImpl::ServiceError()
{
    common::AutoLock autolock(state_lock_);
    state_ = service::ERROR;
    state_time_.Now();
    Info("Service %s now is in error state.", name_.c_str());
    return true;
}

bool ServiceAgentImpl::ServiceAboutToPause()
{
    return TryTransmitServiceState(service::WORKING, service::ABOUT_TO_PAUSE, &service::IService::OnAboutToPause);
}

bool ServiceAgentImpl::ServicePausing()
{
    return TryTransmitServiceState(service::ABOUT_TO_PAUSE, service::PAUSING, &service::IService::OnPausing);
}

bool ServiceAgentImpl::ServiceResuming()
{
    static const service::ServiceState from[] =
    {
        service::ABOUT_TO_PAUSE,
        service::PAUSING,
    };
    return TryTransmitServiceState(sizeof(from)/sizeof(from[0]), from, service::WORKING, &service::IService::OnResuming);
}

bool ServiceAgentImpl::ServiceAboutToRetire()
{
    static const service::ServiceState from[] =
    {
        service::WORKING,
        service::ABOUT_TO_PAUSE,
        service::PAUSING,
    };
    return TryTransmitServiceState(sizeof(from)/sizeof(from[0]), from, service::ABOUT_TO_RETIRE, &service::IService::OnAboutToRetire);
}

bool ServiceAgentImpl::ServiceRetiring()
{
    return TryTransmitServiceState(service::ABOUT_TO_RETIRE, service::RETIRING, &service::IService::OnRetiring);
}

bool ServiceAgentImpl::ServiceRetired()
{
    return TryTransmitServiceState(service::RETIRING, service::RETIRED, &service::IService::OnRetired);
}

bool ServiceAgentImpl::TryTransmitServiceState(
        service::ServiceState from,
        service::ServiceState to, StateTransmissionCallback callback)
{
    common::AutoLock autolock(state_lock_);
    if (state_ == from)
    {
        state_ = to;
        state_time_.Now();
        Info("Service %s now is in %s state.", name_.c_str(), state_names[to]);
        if (callback != nullptr)
            (service_->*callback)();
        return true;
    }

    return false;
}

bool ServiceAgentImpl::TryTransmitServiceState(
        unsigned int count, const service::ServiceState from[],
        service::ServiceState to, StateTransmissionCallback callback)
{
    common::AutoLock autolock(state_lock_);
    for (unsigned int i=0; i<count; ++i)
    {
        if (state_ == from[i])
        {
            state_ = to;
            state_time_.Now();
            Info("Service %s now is in %s state.", name_.c_str(), state_names[to]);
            if (callback != nullptr)
                (service_->*callback)();
            return true;
        }
    }

    return false;
}

void ServiceAgentImpl::RegularStateMaintenance()
{
    switch (state_)
    {
    case service::WORKING:
        if (service_->IsOverload())
            ServiceAboutToPause();
        break;

    case service::ABOUT_TO_PAUSE:
        if (service_->IsUnderload())
            ServiceResuming();
        else if (state_time_ + runtime_env::TimeSpan(kStateTransmitDelayPeriod, 0) < runtime_env::ABSTime())
            ServicePausing();
        break;

    case service::PAUSING:
        if (service_->IsUnderload())
            ServiceResuming();
        break;

    case service::ABOUT_TO_RETIRE:
        if (state_time_ + runtime_env::TimeSpan(kStateTransmitDelayPeriod, 0) < runtime_env::ABSTime())
            ServiceRetiring();
        break;

    case service::RETIRING:
        if (GetInQueueSize() == 0 && GetInQueueDoingCount() == 0 &&
            GetOutQueueSize() == 0 && GetOutQueueDoingCount() == 0)
        {
            ServiceRetired();
        }
        break;

    default:
        break;
    }
}

unsigned long ServiceAgentImpl::GetInQueueSize()
{
    unsigned long size = 0;

    common::AutoReadLock autolock(slots_rwlock_);
    for (queues_t::const_iterator iter = in_queues_.begin();
        iter != in_queues_.end(); ++iter)
    {
        size += (**iter).GetSize();
    }

    return size;
}

unsigned long ServiceAgentImpl::GetInQueueDoingCount()
{
    unsigned long count = 0;

    common::AutoReadLock autolock(slots_rwlock_);
    for (queues_t::const_iterator iter = in_queues_.begin();
        iter != in_queues_.end(); ++iter)
    {
        count += (**iter).GetDoingCount();
    }

    return count;
}

unsigned long ServiceAgentImpl::GetInQueueTraffic()
{
    unsigned long traffic = 0;

    common::AutoReadLock autolock(slots_rwlock_);
    for (queues_t::const_iterator iter = in_queues_.begin();
        iter != in_queues_.end(); ++iter)
    {
        traffic += (**iter).GetTraffic();
    }

    return traffic;
}

unsigned long ServiceAgentImpl::GetOutQueueSize()
{
    unsigned long size = 0;

    common::AutoReadLock autolock(slots_rwlock_);
    for (queues_t::const_iterator iter = out_queues_.begin();
        iter != out_queues_.end(); ++iter)
    {
        size += (**iter).GetSize();
    }

    return size;
}

unsigned long ServiceAgentImpl::GetOutQueueDoingCount()
{
    unsigned long count = 0;

    common::AutoReadLock autolock(slots_rwlock_);
    for (queues_t::const_iterator iter = out_queues_.begin();
        iter != out_queues_.end(); ++iter)
    {
        count += (**iter).GetDoingCount();
    }

    return count;
}

unsigned long ServiceAgentImpl::GetOutQueueTraffic()
{
    unsigned long traffic = 0;

    common::AutoReadLock autolock(slots_rwlock_);
    for (queues_t::const_iterator iter = out_queues_.begin();
        iter != out_queues_.end(); ++iter)
    {
        traffic += (**iter).GetTraffic();
    }

    return traffic;
}

int ServiceAgentImpl::MaintThreadProc(void* arg)
{
    ServiceAgentImpl *agent = reinterpret_cast<ServiceAgentImpl*>(arg);
    return agent->MaintThreadProc();
}

int ServiceAgentImpl::MaintThreadProc()
{
    if (!RegisterDelegates())
        return 1;

    Info("Initializing service %s ...", name_.c_str());
    if (!service_->Initialize())
    {
        Error("Failed to initialize service %s", name_.c_str());
        ServiceError();
        return 1;
    }
    Info("Service %s initialized", name_.c_str());

    if (!StartProcessors())
        return 1;
    if (!StartTimers())
        return 1;

    if (!ServiceInitialized())
        return 1;

    while (state_ != service::RETIRED)
    {
        RegularStateMaintenance();

        if (maint_thread_quit_event_->TimedWait(kMaintStatusInterval))
            break;
    }

    StopTimers();
    StopProcessors();
    CleanupSlots();

    Info("Finalizing service %s ...", name_.c_str());
    service_->Finalize();
    Info("Service %s finalized", name_.c_str());

    CleanupQueues();

    delete service_;
    service_ = nullptr;

    state_ = service::DYING;
    return 0;
}

bool ServiceAgentImpl::CheckSlotNameValidity(const char* name)
{
    // slot name should not contain some charactors which have special
    // meaning in HTTP URL
    return nullptr == strpbrk(name, "/?#");
}

ResponseCode ServiceAgentImpl::ServiceAccessibleForRequest(INetRequest *request, SlotHandler *handler)
{
    assert(request != nullptr && handler != nullptr);

    switch (state_)
    {
    case service::BORNING:
        Info("Service %s is still borning; request denied", name_.c_str());
        return RC_SERVICE_BUSY;

    case service::WAITING_CONFIG:
        if (0 == strncmp("config/", request->GetURL(), 7))
            return RC_OK;
        Info("Service %s is waiting for configurations; only configurations allowed", name_.c_str());
        return RC_SERVICE_BUSY;

    case service::CONFIGURING:
        Info("Service %s is configuring itself; request denied", name_.c_str());
        return RC_SERVICE_BUSY;

    case service::WORKING:
    case service::ABOUT_TO_PAUSE:
    case service::ABOUT_TO_RETIRE:
        return RC_OK;

    case service::PAUSING:
        if (nullptr == dynamic_cast<PushSlotHandler*>(handler))
            return RC_OK;
        Info("Service %s is pausing; PUSH request denied", name_.c_str());
        return RC_SERVICE_BUSY;

    case service::RETIRING:
        if (nullptr == dynamic_cast<PushSlotHandler*>(handler))
            return RC_OK;
        Info("Service %s is retiring; PUSH request denied", name_.c_str());
        return RC_SERVICE_BUSY;

    case service::RETIRED:
        Info("Service %s is retried; request denied", name_.c_str());
        return RC_SERVICE_BUSY;

    case service::DYING:
        Info("Service %s is dying; request denied", name_.c_str());
        return RC_SERVICE_BUSY;

    case service::ERROR:
        Info("Service %s is in ERROR state; request denied", name_.c_str());
        return RC_SERVICE_BUSY;

    default:
        Error("Service %s is in unknown state; request denied", name_.c_str());
        return RC_SERVICE_ERROR;
    }
}

bool ServiceAgentImpl::RegisterSlot(const char* name, const char* method, const char* type, SlotHandler *handler)
{
    std::string slot_name(name);
    std::unique_ptr<SlotHandler> slot_handler(handler);
    resource_method_pair_t pair(name, method);

    common::AutoWriteLock autolock(slots_rwlock_);
    if (slot_handlers_.find(pair) != slot_handlers_.end())
    {
        Error("Slot %s /%s/%s already registered", method, name_.c_str(), slot_name.c_str());
        return false;
    }

    slot_handlers_[pair] = slot_handler.release();
    Info("%s-slot %s /%s/%s registered", type, method, name_.c_str(), slot_name.c_str());
    return true;
}

SlotHandler* ServiceAgentImpl::LookupSlotHandler_NoLock(INetRequest *request)
{
    assert(request != nullptr);

    const char* url = request->GetURL();
    const char* method = request->GetMethod();

    assert(url[0] == '/');
    const char* second_slash = strchr(url+1, '/');
    assert(second_slash != nullptr);
    const char* question_mark = strchr(second_slash+1, '?');

    const std::string resource(second_slash+1, question_mark == nullptr ? strlen(second_slash+1) : question_mark-second_slash-1);
    resource_method_pair_t pair(resource, method);
    slot_handlers_t::const_iterator iter = slot_handlers_.find(pair);
    if (iter == slot_handlers_.end())
    {
        Error("Slot %s %.*s not found", method, second_slash+1-url + resource.size(), url);
        return nullptr;
    }

    assert(iter->second != nullptr);
    return iter->second;
}

void ServiceAgentImpl::CleanupSlots()
{
    common::AutoWriteLock autolock(slots_rwlock_);
    for (slot_handlers_t::iterator iter = slot_handlers_.begin();
        iter != slot_handlers_.end(); ++iter)
    {
        SlotHandler *handler = iter->second;
        delete handler;
    }
    slot_handlers_.clear();

    Info("Slots of service %s cleaned", name_.c_str());
}

bool ServiceAgentImpl::StartProcessors()
{
    common::AutoWriteLock autolock(slots_rwlock_);

    for (task_processors_t::iterator iter = processors_.begin();
        iter != processors_.end(); ++iter)
    {
        task_processor_t &processor = iter->second;
        if (!processor.processor->Start(
                    *processor.queue,
                    processor.concurrency,
                    service_,
                    processor.handler))
        {
            Error("Failed to start processor %s", iter->first.c_str());
            return false;
        }
    }

    return true;
}

void ServiceAgentImpl::StopProcessors()
{
    common::AutoWriteLock autolock(slots_rwlock_);

    for (task_processors_t::iterator iter = processors_.begin();
        iter != processors_.end(); ++iter)
    {
        task_processor_t &processor = iter->second;
        processor.processor->Stop();
    }

    processors_.clear();
}

bool ServiceAgentImpl::StartTimers()
{
    common::AutoWriteLock autolock(slots_rwlock_);

    for (timers_t::iterator iter = timers_.begin();
        iter != timers_.end(); ++iter)
    {
        timer_t &timer = iter->second;
        if (!timer.scheduler->Start(
                    timer.interval,
                    reinterpret_cast<void(*)(void*)>(timer.handler),
                    static_cast<void*>(service_)))
        {
            Error("Failed to start timer %s", iter->first.c_str());
            return false;
        }
    }

    return true;
}

void ServiceAgentImpl::StopTimers()
{
    common::AutoWriteLock autolock(slots_rwlock_);

    for (timers_t::iterator iter = timers_.begin();
        iter != timers_.end(); ++iter)
    {
        timer_t &timer = iter->second;
        timer.scheduler->Stop();
    }

    timers_.clear();
}

void ServiceAgentImpl::CleanupQueues()
{
    const struct
    {
        queues_t &queues;
        const char* type;
    } qs[] =
    {
        { in_queues_,  "in-queue"  },
        { out_queues_, "out-queue" },
    };

    for (unsigned int i=0; i<sizeof(qs)/sizeof(qs[0]); ++i)
    {
        for (queues_t::iterator iter = qs[i].queues.begin();
            iter != qs[i].queues.end(); ++iter)
        {
            std::shared_ptr<QueueImpl> queue(*iter);

            const unsigned long size = queue->GetSize();
            if (size== 0)
                continue;
            Info("There are still %d tasks in %s /%s/%s",
                size, qs[i].type, GetServiceName(), queue->GetName());

            unsigned long count = 0;
            while (true)
            {
                service::ITask *task = queue->GetTask(false, 0);
                if (task == nullptr)
                    break;
                delete task;
                ++count;
            }
            queue->TaskDone(count);
        }

        qs[i].queues.clear();
    }
}

bool ServiceAgentImpl::RegisterDelegates()
{
    if (!RegisterSlot("@retire", "POST", "Update",
                new DelegatedUpdateHandler(this, &ServiceAgentImpl::RetireService)))
        return false;

    return true;
}

bool ServiceAgentImpl::RetireService(service::IRequest*, service::ITask**)
{
    return ServiceAboutToRetire();
}

}}  // namespace fasmio::container

