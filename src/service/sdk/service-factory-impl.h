
#ifndef SERVICE_SDK_SERVICE_FACTORY_IMPL_H_
#define SERVICE_SDK_SERVICE_FACTORY_IMPL_H_

#include "./service-factory.h"
#include "service/interface/plugin.h"
#include <map>
#include <string>
#include <vector>

namespace fasmio { namespace service { namespace sdk {

class ServiceFactoryImpl : public ServiceFactory
{
    friend class ServiceFactory;

protected:
    ServiceFactoryImpl();

public:
    virtual bool RegisterService(const char* name, ServiceCreator creator);
    virtual bool RegisterGlobalInitializer(GlobalInitializer initializer);
    virtual bool RegisterGlobalFinalizer(GlobalFinalizer finalizer);

public:
    void InvokeGlobalInitializers();
    void InvokeGlobalFinalizers();
    void RegisterServices(fasmio::service::IPlugin *plugin);

private:
    typedef std::map<std::string, ServiceCreator> creators_t;
    creators_t creators_;

    typedef std::vector<GlobalInitializer> initializers_t;
    initializers_t initializers_;

    typedef std::vector<GlobalFinalizer> finalizers_t;
    finalizers_t finalizers_;
};

}}}  // namespace fasmio::service::sdk

#endif  // SERVICE_SDK_SERVICE_FACTORY_IMPL_H_

