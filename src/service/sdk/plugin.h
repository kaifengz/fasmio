
#ifndef SERVER_SDK_PLUGIN_H_
#define SERVER_SDK_PLUGIN_H_

#include "service/interface/plugin.h"

#ifdef WIN32
#   define EXPORT_API  _declspec(dllexport)
#else
#   define EXPORT_API  __attribute__((visibility("default")))
#endif

extern "C" {

EXPORT_API unsigned int GetSDKVersion();
EXPORT_API const char* GetPluginVersion();
EXPORT_API bool InitializePlugin(fasmio::service::IPlugin *plugin);
EXPORT_API void FinalizePlugin();

}  // extern "C"

#endif  // SERVER_SDK_PLUGIN_H_

