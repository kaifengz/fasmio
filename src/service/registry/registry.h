
#ifndef SERVICE_REGISTRY_REGISTRY_H_
#define SERVICE_REGISTRY_REGISTRY_H_

#include "service/sdk/service-base.h"
#include "service/sdk/general-xsd-task.h"
#include "./data-types.xsd.h"
#include <map>
#include <memory>
#include <string>

namespace fasmio { namespace service { namespace registry {

typedef sdk::GeneralXsdTask<node_info_t>   NodeTask;
typedef sdk::GeneralXsdTask<nodes_info_t>  NodesTask;

class Registry : public sdk::ServiceBase
{
public:
    Registry(const char* name, IServiceAgent *agent, IRuntimeEnv* env, ILogger *logger);

public:
    virtual bool OnInitialize ();
    virtual void OnFinalize   ();

public:
    bool DoHeartBeat   (IRequest*, NodeTask*, NodesTask**);
    bool DoUnregister  (IRequest*, NodeTask*, NodesTask**);
    bool DoGetNodes    (IRequest*, NodesTask**);
    void MaintainNodes ();

private:
    bool heart_beat(NodeTask*);
    bool unregister(NodeTask*);
    bool get_all_nodes(NodesTask**);

private:
    typedef std::map<std::string, node_info_t> nodes_t;
    nodes_t nodes_;
    std::unique_ptr<runtime_env::IRWLock> nodes_lock_;
};

}}}  // namespace fasmio::service::registry

#endif  // SERVICE_REGISTRY_REGISTRY_H_

