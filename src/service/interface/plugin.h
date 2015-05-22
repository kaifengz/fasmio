
#ifndef SERVICE_INTERFACE_PLUGIN_H_
#define SERVICE_INTERFACE_PLUGIN_H_

#include "./service.h"
#include "./service-agent.h"
#include "interface/logger.h"
#include "interface/runtime-env.h"

namespace fasmio { namespace service {

typedef IService* (*ServiceCreator)(const char* name, IServiceAgent* agent, IRuntimeEnv* env, ILogger *logger);

class IPlugin
{
public:
    virtual ~IPlugin() {}

public:
    virtual bool RegisterService(const char* name, ServiceCreator creator) = 0;
};

}}  // namespace fasmio::service

#endif  // SERVICE_INTERFACE_PLUGIN_H_

