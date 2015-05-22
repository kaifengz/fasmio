
#ifndef SERVICE_SDK_SERVICE_BASE_H_
#define SERVICE_SDK_SERVICE_BASE_H_

#include "./service-registor.h"
#include "interface/runtime-env.h"
#include "interface/logger.h"
#include "common/module-logger.h"
#include "service/interface/service.h"
#include "service/interface/service-agent.h"
#include "service/interface/queue.h"
#include <memory>
#include <string>

namespace fasmio { namespace service { namespace sdk {

class ServiceBase : public IService
{
public:
    ServiceBase(const char* name, IServiceAgent *agent, IRuntimeEnv* env, ILogger *logger);
    virtual ~ServiceBase();

public:
    virtual bool OnInitialize () = 0;
    virtual void OnFinalize   () = 0;

public:
    virtual bool Initialize ();
    virtual void Finalize   ();

public:
    virtual void OnConfiguring   () {}
    virtual void OnConfigured    () {}
    virtual void OnAboutToPause  () {}
    virtual void OnPausing       () {}
    virtual void OnResuming      () {}
    virtual void OnAboutToRetire () {}
    virtual void OnRetiring      () {}
    virtual void OnRetired       () {}

public:
    virtual bool IsOverload  ();
    virtual bool IsUnderload ();

public:
    const char*           GetName         () { return name_.c_str(); }
    IServiceAgent*        GetAgent        () { return agent_;        }
    ILogger*              GetLogger       () { return logger_;       }
    common::ModuleLogger& GetModuleLogger () { return mlogger_;      }

public:
    void Verbose (const char* format, ...);
    void Info    (const char* format, ...);
    void Error   (const char* format, ...);

public:
    template < typename ServiceClass,
               typename InTaskClass,
               void (ServiceClass::*PushSlotHandler)(std::unique_ptr<InTaskClass>)>
    bool RegisterPushSlot(const char* name, TaskAllocator in_task_allocator, unsigned concurrency);

    bool RegisterPopSlot(const char* name, IOutQueue **queue);

    template < typename ServiceClass,
               typename OutTaskClass,
               bool (ServiceClass::*QuerySlotHandler)(IRequest*, OutTaskClass**)>
    bool RegisterQuerySlot(const char* name);

    template < typename ServiceClass,
               typename InTaskClass,
               typename OutTaskClass,
               bool (ServiceClass::*UpdateSlotHandler)(IRequest*, InTaskClass*, OutTaskClass**)>
    bool RegisterUpdateSlot(const char* name, TaskAllocator in_task_allocator);

    template < typename ServiceClass,
               bool (ServiceClass::*DeleteSlotHandler)(IRequest*)>
    bool RegisterDeleteSlot(const char* name);

    template < typename ServiceClass,
               void (ServiceClass::*TimerHandler)()>
    bool RegisterTimer(const char* name, unsigned long interval);

public:
    // agent API shortcuts

    inline ServiceState  GetServiceState        () { return agent_->GetServiceState        (); }
    inline bool          ServiceConfiguring     () { return agent_->ServiceConfiguring     (); }
    inline bool          ServiceConfigured      () { return agent_->ServiceConfigured      (); }
    inline bool          ServiceError           () { return agent_->ServiceError           (); }
    inline unsigned long GetInQueueSize         () { return agent_->GetInQueueSize         (); }
    inline unsigned long GetInQueueDoingCount   () { return agent_->GetInQueueDoingCount   (); }
    inline unsigned long GetInQueueTraffic      () { return agent_->GetInQueueTraffic      (); }
    inline unsigned long GetOutQueueSize        () { return agent_->GetOutQueueSize        (); }
    inline unsigned long GetOutQueueDoingCount  () { return agent_->GetOutQueueDoingCount  (); }
    inline unsigned long GetOutQueueTraffic     () { return agent_->GetOutQueueTraffic     (); }

protected:
    bool           SetFlowControlMinThroughput  (unsigned long); // default to 1000
    bool           SetFlowControlOverloadCoeff  (double);        // default to 1.0
    bool           SetFlowControlUnderloadCoeff (double);        // default to 0.8
    unsigned long  GetFlowControlMinThroughput  () const { return min_throughput_;  }
    double         GetFlowControlOverloadCoeff  () const { return overload_coeff_;  }
    double         GetFlowControlUnderloadCoeff () const { return underload_coeff_; }

private:
    std::string const     name_;
    IServiceAgent* const  agent_;
    ILogger* const        logger_;
    common::ModuleLogger  mlogger_;

    unsigned long   min_throughput_;
    double          overload_coeff_;
    double          underload_coeff_;

protected:
    IRuntimeEnv * const   env_;
};

}}}  // namespace fasmio::service::sdk

#include "./service-base-aux.h"

#endif  // SERVICE_SDK_SERVICE_BASE_H_

