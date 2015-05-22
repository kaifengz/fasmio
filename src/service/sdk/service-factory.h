
#ifndef SERVICE_SDK_SERVICE_FACTORY_H_
#define SERVICE_SDK_SERVICE_FACTORY_H_

#include "interface/runtime-env.h"
#include "service/interface/service.h"

namespace fasmio { namespace service { namespace sdk {

typedef IService* (*ServiceCreator)(const char* name, IServiceAgent* agent, IRuntimeEnv* env, ILogger *logger);
typedef void      (*GlobalInitializer) ();
typedef void      (*GlobalFinalizer)   ();

class ServiceFactory
{
public:
    static ServiceFactory* GetInstance();

public:
    virtual ~ServiceFactory() {}
    virtual bool RegisterService(const char* name, ServiceCreator creator) = 0;
    virtual bool RegisterGlobalInitializer(GlobalInitializer initializer) = 0;
    virtual bool RegisterGlobalFinalizer(GlobalFinalizer finalizer) = 0;
};


}}}  // namespace fasmio::service::sdk

#endif  // SERVICE_SDK_SERVICE_FACTORY_H_

