
#ifndef CONTAINER_SERVICE_AGENT_IMPL_H_
#define CONTAINER_SERVICE_AGENT_IMPL_H_

#include "common/module-logger.h"
#include "common/task-processor.h"
#include "common/task-scheduler.h"
#include "interface/container.h"
#include "interface/logger.h"
#include "interface/runtime-env.h"
#include "service/interface/service.h"
#include "service/interface/service-agent.h"
#include "./plugin-impl.h"
#include "./queue-impl.h"
#include <memory>
#include <string>

namespace fasmio { namespace container {

class SlotHandler;

class ServiceAgentImpl : public service::IServiceAgent
{
public:
    explicit ServiceAgentImpl(IRuntimeEnv* env, ILogger* logger, const char* name);
    virtual ~ServiceAgentImpl();

public:
    bool InitializeAsynchronously(service::IService *srv);
    void Finalize();

    ResponseCode PrepareRequest(INetRequest*);
    ResponseCode ServeRequest(INetRequest*, IOStream*);

    void Verbose (const char* format, ...);
    void Info    (const char* format, ...);
    void Error   (const char* format, ...);

    service::IService* GetService();
    const char*        GetServiceName();

public:
    virtual service::ServiceState GetServiceState();

public:
    virtual bool RegisterPushSlot     (const char* name, service::TaskAllocator alloc, service::PushSlotHandler handler, unsigned concurrency);
    virtual bool RegisterPopSlot      (const char* name, service::IOutQueue **queue);
    virtual bool RegisterQuerySlot    (const char* name, service::QuerySlotHandler handler);
    virtual bool RegisterUpdateSlot   (const char* name, service::TaskAllocator alloc, service::UpdateSlotHandler handler);
    virtual bool RegisterDeleteSlot   (const char* name, service::DeleteSlotHandler handler);

    virtual bool RegisterTimer        (const char* name, unsigned long interval, service::TimerHandler handler);

public:
    virtual bool NeedConfiguration    (const char* configuration);

public:
    virtual bool ServiceConfiguring   ();
    virtual bool ServiceConfigured    ();
    virtual bool ServiceError         ();
            bool ServiceInitialized   ();
            bool ServiceAboutToPause  ();
            bool ServicePausing       ();
            bool ServiceResuming      ();
            bool ServiceAboutToRetire ();
            bool ServiceRetiring      ();
            bool ServiceRetired       ();

public:
    virtual unsigned long GetInQueueSize         ();
    virtual unsigned long GetInQueueDoingCount   ();
    virtual unsigned long GetInQueueTraffic      ();
    virtual unsigned long GetOutQueueSize        ();
    virtual unsigned long GetOutQueueDoingCount  ();
    virtual unsigned long GetOutQueueTraffic     ();

private:
    static int MaintThreadProc(void*);
    int MaintThreadProc();

    bool CheckSlotNameValidity(const char* name);

    ResponseCode ServiceAccessibleForRequest(INetRequest *request, SlotHandler *handler);

    bool RegisterSlot(const char* name, const char* method, const char* type, SlotHandler *handler);

    SlotHandler* LookupSlotHandler_NoLock(INetRequest *request);

    void CleanupSlots();

private:
    bool StartProcessors();
    void StopProcessors();

    bool StartTimers();
    void StopTimers();

    void CleanupQueues();

private:
    bool RegisterDelegates();
    bool RetireService(service::IRequest*, service::ITask**);

private:
    typedef void (service::IService::*StateTransmissionCallback)();
    bool TryTransmitServiceState(
            service::ServiceState from,
            service::ServiceState to, StateTransmissionCallback);
    bool TryTransmitServiceState(
            unsigned int count, const service::ServiceState from[],
            service::ServiceState to, StateTransmissionCallback);

    void RegularStateMaintenance();

private:
    typedef std::unique_ptr<runtime_env::IEvent> event_ptr_t;
    typedef std::unique_ptr<runtime_env::IMutex> mutex_ptr_t;
    typedef std::unique_ptr<runtime_env::IRWLock> rwlock_ptr_t;
    typedef std::pair<std::string, std::string> resource_method_pair_t;
    typedef std::map<resource_method_pair_t, SlotHandler*> slot_handlers_t;

    typedef std::shared_ptr<QueueImpl> queue_ptr_t;
    typedef std::vector<queue_ptr_t> queues_t;

    struct task_processor_t
    {
        std::shared_ptr<common::TaskProcessor<service::IService, service::ITask, QueueImpl> > processor;
        unsigned int concurrency;
        queue_ptr_t queue;
        service::PushSlotHandler handler;
    };
    typedef std::map<std::string, task_processor_t> task_processors_t;

    struct timer_t
    {
        std::shared_ptr<common::TaskScheduler> scheduler;
        unsigned long interval;
        service::TimerHandler handler;
    };
    typedef std::map<std::string, timer_t> timers_t;

    IRuntimeEnv              *env_;
    ILogger                  *logger_;
    std::string               name_;
    common::ModuleLogger      mlogger_;

    service::IService        *service_;

    service::ServiceState     state_;
    runtime_env::ABSTime      state_time_;
    mutex_ptr_t               state_lock_;

    runtime_env::IThread     *maint_thread_;
    event_ptr_t               maint_thread_quit_event_;

    rwlock_ptr_t              slots_rwlock_;
    slot_handlers_t           slot_handlers_;
    queues_t                  in_queues_;
    queues_t                  out_queues_;
    task_processors_t         processors_;
    timers_t                  timers_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_SERVICE_AGENT_IMPL_H_

