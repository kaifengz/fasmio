
#ifndef SERVICE_SDK_SERVICE_BASE_H_
#   error Do not include "service-base-aux.h" directly, include "service-base.h" instead
#endif

#ifndef SERVICE_SDK_SERVICE_BASE_AUX_H_
#define SERVICE_SDK_SERVICE_BASE_AUX_H_

namespace fasmio { namespace service { namespace sdk {

namespace detail {

    // PushSlotAdaptor
    template < typename ServiceClass,
               typename InTaskClass,
               void (ServiceClass::*PushSlotHandler)(std::unique_ptr<InTaskClass>)
             >
    void PushSlotAdaptor(IService* service, ITask* in_task)
    {
        ServiceClass *svr = static_cast<ServiceClass*>(service);
        InTaskClass *in_tsk = static_cast<InTaskClass*>(in_task);
        (svr->*PushSlotHandler)(std::unique_ptr<InTaskClass>(in_tsk));
    }

    // QuerySlotAdaptor
    template < typename ServiceClass,
               typename OutTaskClass,
               bool (ServiceClass::*QuerySlotHandler)(IRequest*, OutTaskClass**)
             >
    bool QuerySlotAdaptor(IService *service, IRequest *request, ITask **out_task)
    {
        ServiceClass *svr = static_cast<ServiceClass*>(service);
        OutTaskClass *out_tsk = nullptr;
        if ( !(svr->*QuerySlotHandler)(request, &out_tsk) )
            return false;
        *out_task = static_cast<ITask*>(out_tsk);
        return true;
    }

    // UpdateSlotAdaptor
    template < typename ServiceClass,
               typename InTaskClass,
               typename OutTaskClass,
               bool (ServiceClass::*UpdateSlotHandler)(IRequest*, InTaskClass*, OutTaskClass**)
             >
    bool UpdateSlotAdaptor(IService *service, IRequest *request, ITask *in_task, ITask **out_task)
    {
        ServiceClass *svr = static_cast<ServiceClass*>(service);
        InTaskClass *in_tsk = static_cast<InTaskClass*>(in_task);
        OutTaskClass *out_tsk = nullptr;
        if ( !(svr->*UpdateSlotHandler)(request, in_tsk, &out_tsk) )
            return false;
        *out_task = static_cast<ITask*>(out_tsk);
        return true;
    }

    // DeleteSlotAdaptor
    template < typename ServiceClass,
               bool (ServiceClass::*DeleteSlotHandler)(IRequest*)
             >
    bool DeleteSlotAdaptor(IService *service, IRequest *request)
    {
        ServiceClass *svr = static_cast<ServiceClass*>(service);
        return (svr->*DeleteSlotHandler)(request);
    }

    // TimerAdaptor
    template < typename ServiceClass,
               void (ServiceClass::*TimerHandler)()
             >
    void TimerAdaptor(IService *service)
    {
        ServiceClass *svr = static_cast<ServiceClass*>(service);
        (svr->*TimerHandler)();
    }

}  // namespace detail


// ServiceBase::RegisterPushSlot
template < typename ServiceClass,
           typename InTaskClass,
           void (ServiceClass::*PushSlotHandler)(std::unique_ptr<InTaskClass>)
         >
bool ServiceBase::RegisterPushSlot(const char* name, TaskAllocator in_task_allocator, unsigned concurrency)
{
    return agent_->RegisterPushSlot(
                    name,
                    in_task_allocator,
                    detail::PushSlotAdaptor<ServiceClass, InTaskClass, PushSlotHandler>,
                    concurrency);
}

// ServiceBase::RegisterQuerySlot
template < typename ServiceClass,
           typename OutTaskClass,
           bool (ServiceClass::*QuerySlotHandler)(IRequest*, OutTaskClass**)
         >
bool ServiceBase::RegisterQuerySlot(const char* name)
{
    return agent_->RegisterQuerySlot(
                    name,
                    detail::QuerySlotAdaptor<ServiceClass, OutTaskClass, QuerySlotHandler>);
}

// ServiceBase::RegisterUpdateSlot
template < typename ServiceClass,
           typename InTaskClass,
           typename OutTaskClass,
           bool (ServiceClass::*UpdateSlotHandler)(IRequest*, InTaskClass*, OutTaskClass**)
         >
bool ServiceBase::RegisterUpdateSlot(const char* name, TaskAllocator in_task_allocator)
{
    return agent_->RegisterUpdateSlot(
                    name,
                    in_task_allocator,
                    detail::UpdateSlotAdaptor<ServiceClass, InTaskClass, OutTaskClass, UpdateSlotHandler>);
}

// ServiceBase::RegisterDeleteSlot
template < typename ServiceClass,
           bool (ServiceClass::*DeleteSlotHandler)(IRequest*)
         >
bool ServiceBase::RegisterDeleteSlot(const char* name)
{
    return agent_->RegisterDeleteSlot(
                    name,
                    detail::DeleteSlotAdaptor<ServiceClass, DeleteSlotHandler>);
}

// ServiceBase::RegisterTime
template < typename ServiceClass,
           void (ServiceClass::*TimerHandler)()
         >
bool ServiceBase::RegisterTimer(const char* name, unsigned long interval)
{
    return agent_->RegisterTimer(
                    name,
                    interval,
                    detail::TimerAdaptor<ServiceClass, TimerHandler>);
}

}}}  // namespace fasmio::service::sdk

#endif  // SERVICE_SDK_SERVICE_BASE_AUX_H_

