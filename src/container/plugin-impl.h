
#ifndef CONTAINER_PLUGIN_IMPL_H_
#define CONTAINER_PLUGIN_IMPL_H_

#include "common/module-logger.h"
#include "service/interface/plugin.h"
#include <map>
#include <string>

namespace fasmio { namespace container {

class PluginImpl : public service::IPlugin
{
public:
    explicit PluginImpl(ILogger* logger, const char* so_name);
    virtual ~PluginImpl();

public:
    typedef std::map<std::string, service::ServiceCreator> service_creator_map_t;
    const service_creator_map_t& DiscoverAvailableServices();

    const char* GetEmptyReason();
    const char* GetPluginVersion();

public:
    virtual bool RegisterService(const char* name, service::ServiceCreator creator);

private:
    bool LoadPlugin();
    void UnloadPlugin();

    bool IsCompatibleSDKVersion(unsigned int plugin_sdk_version);
    void SetEmptyReason(const char* format, ...);

private:
    ILogger* const logger_;
    common::ModuleLogger mlogger_;

    std::string so_name_;
    void* so_handle_;
    void* finalize_plugin_;

    std::string empty_reason_;
    std::string plugin_version_;

    service_creator_map_t creators_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_PLUGIN_IMPL_H_

