
#include "./service-factory-impl.h"
#include <map>
#include <string>

namespace fasmio { namespace service { namespace sdk {

ServiceFactoryImpl::ServiceFactoryImpl() :
    creators_(),
    initializers_(),
    finalizers_()
{
}

ServiceFactory* ServiceFactory::GetInstance()
{
    static ServiceFactoryImpl factory;
    return &factory;
}

bool ServiceFactoryImpl::RegisterService(const char* name, ServiceCreator creator)
{
    if (name == nullptr || creator == nullptr)
        return false;

    std::string sname(name);
    if (creators_.find(sname) != creators_.end())
        return false;

    creators_[sname] = creator;
    return true;
}

bool ServiceFactoryImpl::RegisterGlobalInitializer(GlobalInitializer initializer)
{
    initializers_.push_back(initializer);
    return true;
}

bool ServiceFactoryImpl::RegisterGlobalFinalizer(GlobalFinalizer finalizer)
{
    finalizers_.push_back(finalizer);
    return true;
}

void ServiceFactoryImpl::InvokeGlobalInitializers()
{
    // invoke all initializers in the register order
    for (initializers_t::iterator iter = initializers_.begin();
        iter != initializers_.end(); ++iter)
    {
        (*iter)();
    }
}

void ServiceFactoryImpl::InvokeGlobalFinalizers()
{
    // invoke all initializers in the reverse order
    for (finalizers_t::reverse_iterator iter = finalizers_.rbegin();
        iter != finalizers_.rend(); ++iter)
    {
        (*iter)();
    }
}

void ServiceFactoryImpl::RegisterServices(fasmio::service::IPlugin *plugin)
{
    if (plugin == nullptr)
        return;

    typedef std::map<std::string, ServiceCreator> creators_t;
    for (creators_t::iterator iter = creators_.begin();
        iter != creators_.end(); ++iter)
    {
        const std::string &name = iter->first;
        ServiceCreator creator = iter->second;
        plugin->RegisterService(name.c_str(), creator);
    }
}

}}}  // namespace fasmio::service::sdk

