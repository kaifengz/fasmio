
#include "./plugin-impl.h"
#include "service/version.h"
#include <dlfcn.h>

#define SYMBOL_NAME_GetSDKVersion      "GetSDKVersion"
#define SYMBOL_NAME_GetPluginVersion   "GetPluginVersion"
#define SYMBOL_NAME_InitializePlugin   "InitializePlugin"
#define SYMBOL_NAME_FinalizePlugin     "FinalizePlugin"

namespace fasmio { namespace container {

PluginImpl::PluginImpl(ILogger* logger, const char* so_name) :
    logger_(logger),
    mlogger_(logger, "PluginImpl"),
    so_name_(so_name),
    so_handle_(nullptr),
    finalize_plugin_(nullptr),
    empty_reason_(),
    plugin_version_(),
    creators_()
{
}

PluginImpl::~PluginImpl()
{
    UnloadPlugin();
}

const PluginImpl::service_creator_map_t& PluginImpl::DiscoverAvailableServices()
{
    UnloadPlugin();

    if (LoadPlugin())
        return creators_;

    UnloadPlugin();
    return creators_;
}

const char* PluginImpl::GetEmptyReason()
{
    return empty_reason_.c_str();
}

const char* PluginImpl::GetPluginVersion()
{
    return plugin_version_.c_str();
}

bool PluginImpl::RegisterService(const char* name, service::ServiceCreator creator)
{
    if (name == nullptr || creator == nullptr)
        return false;

    const std::string sname(name);
    if (creators_.find(sname) != creators_.end())
        return false;

    creators_[sname] = creator;
    return true;
}

bool PluginImpl::LoadPlugin()
{
    if (nullptr == (so_handle_ = dlopen(so_name_.c_str(), RTLD_NOW)))
    {
        const char* reason = dlerror();
        SetEmptyReason("%s", reason);
        mlogger_.Error("Cannot load %s: %s", so_name_.c_str(), reason);
        return false;
    }

    static const char* symbol_names[] =
    {
        "GetSDKVersion",
        "GetPluginVersion",
        "InitializePlugin",
        "FinalizePlugin",
    };
    const int symbol_count = sizeof(symbol_names)/sizeof(symbol_names[0]);
    void* symbols[symbol_count];

    for (int i=0; i<symbol_count; ++i)
    {
        if (nullptr == (symbols[i] = dlsym(so_handle_, symbol_names[i])))
        {
            SetEmptyReason("Symbol %s not found", symbol_names[i]);
            mlogger_.Error("Cannot locate symbol %s in %s",
                    symbol_names[i], so_name_.c_str());
            return false;
        }
    }

    typedef unsigned int (*GetSDKVersion_Proc)();
    GetSDKVersion_Proc GetSDKVersion = reinterpret_cast<GetSDKVersion_Proc>(symbols[0]);
    unsigned int plugin_sdk_version = GetSDKVersion();
    if (!IsCompatibleSDKVersion(plugin_sdk_version))
    {
        SetEmptyReason("Plugin SDK version %d is incompatible with Container version %d",
                plugin_sdk_version, FASMIO_SDK_VERSION);
        mlogger_.Error("SDK version incompatible, Container version %d, %s version %d",
                FASMIO_SDK_VERSION, so_name_.c_str(), plugin_sdk_version);
        return false;
    }

    typedef const char* (*GetPluginVersion_Proc)();
    GetPluginVersion_Proc GetPluginVersion = reinterpret_cast<GetPluginVersion_Proc>(symbols[1]);
    const char* plugin_version = GetPluginVersion();
    if (nullptr == plugin_version)
        plugin_version_.clear();
    else
        plugin_version_ = plugin_version;

    typedef bool (*InitializePlugin_Proc)(IPlugin*);
    InitializePlugin_Proc InitializePlugin = reinterpret_cast<InitializePlugin_Proc>(symbols[2]);
    if (!InitializePlugin(static_cast<IPlugin*>(this)))
    {
        SetEmptyReason("Error initializing the plugin");
        mlogger_.Error("Error initializing plugin %s", so_name_.c_str());
        return false;
    }

    finalize_plugin_ = symbols[3];

    if (creators_.empty())
        SetEmptyReason("No service found");
    else
        SetEmptyReason(nullptr);

    return true;
}

void PluginImpl::UnloadPlugin()
{
    if (finalize_plugin_ != nullptr)
    {
        typedef void (*FinalizePlugin_Proc)();
        FinalizePlugin_Proc FinalizePlugin = reinterpret_cast<FinalizePlugin_Proc>(finalize_plugin_);
        FinalizePlugin();

        finalize_plugin_ = nullptr;
    }

    if (so_handle_ != nullptr)
    {
        dlclose(so_handle_);
        so_handle_ = nullptr;
    }

    empty_reason_.clear();
    plugin_version_.clear();
    creators_.clear();
}

bool PluginImpl::IsCompatibleSDKVersion(unsigned int plugin_sdk_version)
{
    int plugin_major = FASMIO_SDK_MAJOR(plugin_sdk_version);
    if (plugin_major <= FASMIO_SDK_VERSION_MAJOR)
        return true;

    return false;
}

void PluginImpl::SetEmptyReason(const char* format, ...)
{
    if (format == nullptr)
        empty_reason_.clear();
    else
    {
        char buff[4096];
        va_list args;
        va_start(args, format);
        vsnprintf(buff, sizeof(buff), format, args);
        va_end(args);

        empty_reason_ = buff;
    }
}

}}  // namespace fasmio::container

