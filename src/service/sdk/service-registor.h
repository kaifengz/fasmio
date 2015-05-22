
#ifndef SERVICE_SDK_SERVICE_REGISTOR_H_
#define SERVICE_SDK_SERVICE_REGISTOR_H_

#include "./service-factory.h"
#include "service/interface/service.h"

namespace fasmio { namespace service { namespace sdk {

// forward declaration
typedef IService* (*ServiceCreator)    (const char* name, IServiceAgent* agent, IRuntimeEnv* env, ILogger *logger);
typedef void      (*GlobalInitializer) ();
typedef void      (*GlobalFinalizer)   ();

#define REGISTER_SERVICE(name, ServiceClass)  \
    static ::fasmio::service::sdk::StaticServiceRegistor<ServiceClass> ServiceClass ## _registor (name)

#define REGISTER_DYNAMIC_SERVICE(loader) \
    static ::fasmio::service::sdk::DynamicServiceRegistor<ServiceClass> ServiceClass ## _dregistor (loader)

#define REGISTER_GLOBAL_INITIALIZER(initializer) \
    static ::fasmio::service::sdk::GlobalInitializerRegistor _GlobalInitializerRegistor(initializer)

#define REGISTER_GLOBAL_FINALIZER(finalizer) \
    static ::fasmio::service::sdk::GlobalFinalizerRegistor _GlobalFinalizerRegistor(finalizer)

// CreateService
template <typename ServiceClass>
IService* CreateService(const char* name, IServiceAgent* agent, IRuntimeEnv* env, ILogger *logger)
{
    return new ServiceClass(name, agent, env, logger);
}

// StaticServiceRegistor
template <typename ServiceClass>
class StaticServiceRegistor
{
public:
    explicit StaticServiceRegistor(const char* name)
    {
        ServiceFactory::GetInstance()->RegisterService(name, CreateService<ServiceClass>);
    }
};

// SpecialServiceRegistor
template <typename ServiceClass>
class SpecialServiceRegistor
{
public:
    bool RegisterService(const char* name)
    {
        return ServiceFactory::GetInstance()->RegisterService(name, CreateService<ServiceClass>);
    }
};

// DynamicServiceRegistor
template <typename ServiceClass>
class DynamicServiceRegistor
{
public:
    typedef void (*ServiceLoader) (SpecialServiceRegistor<ServiceClass> *registor);

    explicit DynamicServiceRegistor(ServiceLoader loader)
    {
        SpecialServiceRegistor<ServiceClass> registor;
        loader(&registor);
    }
};

// GlobalInitializerRegistor
class GlobalInitializerRegistor
{
public:
    explicit GlobalInitializerRegistor(GlobalInitializer initializer)
    {
        ServiceFactory::GetInstance()->RegisterGlobalInitializer(initializer);
    }
};

// GlobalFinalizerRegistor
class GlobalFinalizerRegistor
{
public:
    explicit GlobalFinalizerRegistor(GlobalFinalizer finalizer)
    {
        ServiceFactory::GetInstance()->RegisterGlobalFinalizer(finalizer);
    }
};

}}}  // namespace fasmio::service::sdk

#endif  // SERVICE_SDK_SERVICE_REGISTOR_H_

