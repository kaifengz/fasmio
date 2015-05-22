
#include "./plugin.h"
#include "./service-factory-impl.h"
#include "service/version.h"

extern "C" {

using fasmio::service::sdk::ServiceFactoryImpl;

EXPORT_API unsigned int GetSDKVersion()
{
    return FASMIO_SDK_VERSION;
}

EXPORT_API const char* GetPluginVersion()
{
    // TODO: GetPluginVersion
    return nullptr;
}

EXPORT_API bool InitializePlugin(fasmio::service::IPlugin *plugin)
{
    ServiceFactoryImpl *factory = static_cast<ServiceFactoryImpl*>(ServiceFactoryImpl::GetInstance());
    if (factory == nullptr)
        return false;

    factory->InvokeGlobalInitializers();
    factory->RegisterServices(plugin);

    return true;
}

EXPORT_API void FinalizePlugin()
{
    ServiceFactoryImpl *factory = static_cast<ServiceFactoryImpl*>(ServiceFactoryImpl::GetInstance());
    if (factory == nullptr)
        return;

    factory->InvokeGlobalFinalizers();
}

}  // extern "C"

