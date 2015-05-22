
#ifndef CONTAINER_CONTAINER_IMPL_H_
#define CONTAINER_CONTAINER_IMPL_H_

#include "common/module-logger.h"
#include "common/task-scheduler.h"
#include "interface/config.h"
#include "interface/container.h"
#include "interface/logger.h"
#include "interface/runtime-env.h"
#include "service/registry/data-types.xsd.h"
#include "./plugin-impl.h"
#include "./service-agent-impl.h"
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace fasmio { namespace container {

class ContainerImpl : public IContainer
{
public:
    explicit ContainerImpl(IRuntimeEnv* env, ILogger* logger);
    virtual ~ContainerImpl();

public:
    virtual bool Initialize(INetClient*, IConfig*);
    virtual void Finalize();

    virtual ResponseCode PrepareRequest(INetRequest*);
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*);

private:
    bool load_plugins();
    bool determine_startup_services(std::vector<std::string> *services);
    bool start_services(const std::vector<std::string> &services);

    unsigned long stop_services(bool stop_dying_only);
    void unload_plugins();

    bool start_service_asynchronously(const std::string &service);

    void delete_agent(const std::string &plugin_name, ServiceAgentImpl *agent);

private:
    struct plugin_ref_t
    {
        PluginImpl* plugin_;
        std::string fullname_;
        std::set<std::string> available_services_;
        unsigned int live_services_;
    };

    struct available_service_t
    {
        std::string plugin_;
        service::ServiceCreator creator_;
    };

    struct agent_deletor_t
    {
        ContainerImpl *container_;
        std::string plugin_;

        agent_deletor_t(ContainerImpl*, const std::string &plugin);
        agent_deletor_t(const agent_deletor_t &);
        void operator() (ServiceAgentImpl*);
    };

    typedef std::map<std::string, plugin_ref_t*>            plugins_t;
    typedef std::map<std::string, available_service_t>      available_services_t;
    typedef std::shared_ptr<ServiceAgentImpl>               agent_ptr_t;
    typedef std::map<std::string, agent_ptr_t>              agents_t;
    typedef std::unique_ptr<runtime_env::IMutex>            mutex_ptr_t;
    typedef std::unique_ptr<runtime_env::IRWLock>           rwlock_ptr_t;

private:
    agent_ptr_t lookup_agent_by_url(const char* url);

private:
    IRuntimeEnv* const env_;
    ILogger* const logger_;
    common::ModuleLogger mlogger_;
    INetClient* net_client_;
    IConfig* config_;

    // if have to lock both plugins_lock_ and agents_lock_, always lock
    // the latter first to avoid deadlock

    plugins_t              plugins_;
    available_services_t   available_services_;
    mutex_ptr_t            plugins_lock_;

    // TODO: still have race condition when destruct agent
    agents_t               agents_;
    rwlock_ptr_t           agents_lock_;

private:
    // configuration related

    bool read_configuration();

    std::string            my_node_name_;
    std::string            my_host_port_;
    std::string            registry_host_port_;

private:
    // heart beat related

    const runtime_env::ABSTime  my_birth_time_;

    typedef common::TaskScheduler task_scheduler_t;
    task_scheduler_t       heart_beat_scheduler_;

    typedef std::shared_ptr<service::registry::nodes_info_t> nodes_info_ptr_t;
    nodes_info_ptr_t nodes_info_;

    void send_heart_beat();
    void unregister_self();

    bool collect_node_info(service::registry::node_info_t *info);
    bool communicate_with_registry(const char* uri);
};

}}  // namespace fasmio::container

#endif  // CONTAINER_CONTAINER_IMPL_H_

