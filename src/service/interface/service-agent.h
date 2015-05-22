
#ifndef SERVICE_INTERFACE_SERVICE_AGENT_H_
#define SERVICE_INTERFACE_SERVICE_AGENT_H_

namespace fasmio { namespace service {

enum ServiceState
{
    BORNING = 0,
    WAITING_CONFIG,
    CONFIGURING,
    WORKING,
    ABOUT_TO_PAUSE,
    PAUSING,
    ABOUT_TO_RETIRE,
    RETIRING,
    RETIRED,
    DYING,
    ERROR,
};

class ITask;
class IInQueue;
class IOutQueue;
class IRequest;
class IService;

typedef ITask*  (*TaskAllocator)     (IService*);
typedef void    (*PushSlotHandler)   (IService*, ITask*);
typedef bool    (*QuerySlotHandler)  (IService*, IRequest*, ITask**);
typedef bool    (*UpdateSlotHandler) (IService*, IRequest*, ITask*, ITask**);
typedef bool    (*DeleteSlotHandler) (IService*, IRequest*);
typedef void    (*TimerHandler)      (IService*);

class IServiceAgent
{
public:
    virtual ~IServiceAgent() {}

public:
    virtual ServiceState GetServiceState() = 0;

public:
    virtual bool RegisterPushSlot     (const char* name, TaskAllocator alloc, PushSlotHandler handler, unsigned concurrency) = 0;
    virtual bool RegisterPopSlot      (const char* name, IOutQueue **queue) = 0;
    virtual bool RegisterQuerySlot    (const char* name, QuerySlotHandler handler) = 0;
    virtual bool RegisterUpdateSlot   (const char* name, TaskAllocator alloc, UpdateSlotHandler handler) = 0;
    virtual bool RegisterDeleteSlot   (const char* name, DeleteSlotHandler handler) = 0;

    virtual bool RegisterTimer        (const char* name, unsigned long interval, TimerHandler handler) = 0;

public:
    virtual bool NeedConfiguration    (const char* configuration) = 0;

public:
    virtual bool ServiceConfiguring   () = 0;
    virtual bool ServiceConfigured    () = 0;
    virtual bool ServiceError         () = 0;

public:
    virtual unsigned long GetInQueueSize        () = 0;
    virtual unsigned long GetInQueueDoingCount  () = 0;
    virtual unsigned long GetInQueueTraffic     () = 0;
    virtual unsigned long GetOutQueueSize       () = 0;
    virtual unsigned long GetOutQueueDoingCount () = 0;
    virtual unsigned long GetOutQueueTraffic    () = 0;
};

}}  // namespace fasmio::service

#endif  // SERVICE_INTERFACE_SERVICE_AGENT_H_

