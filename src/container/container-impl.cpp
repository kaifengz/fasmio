
#include "./container-impl.h"
#include "./stream-impl.h"
#include "common/auto-lock.h"
#include "common/list-dir.h"
#include <assert.h>
#include <string.h>

namespace fasmio { namespace container {

const char* const kConfigSection = "container";
const char* const kDefaultPluginDirectory = "plugin";
const char* const kPluginExtension = ".so";
const unsigned int kDefaultHeartBeatInterval = 2000;

ContainerImpl::ContainerImpl(IRuntimeEnv* env, ILogger* logger) :
    env_(env),
    logger_(logger),
    mlogger_(logger, "Container"),
    net_client_(nullptr),
    config_(nullptr),
    plugins_(),
    available_services_(),
    plugins_lock_(env->NewMutex(false)),
    agents_(),
    agents_lock_(env->NewRWLock()),
    my_node_name_(),
    my_host_port_(),
    registry_host_port_(),
    my_birth_time_(),
    heart_beat_scheduler_(env, logger, "heart-beat-thr"),
    nodes_info_()
{
}

ContainerImpl::~ContainerImpl()
{
}

bool ContainerImpl::Initialize(INetClient *net_client, IConfig *config)
{
    if (net_client == nullptr || config == nullptr)
        return false;
    net_client_ = net_client;
    config_ = config;

    mlogger_.Info("Initializing Container ...");

    if (!read_configuration())
        return false;
    if (!load_plugins())
        return false;

    std::vector<std::string> startup_services;
    if (!determine_startup_services(&startup_services))
        return false;
    if (!start_services(startup_services))
        return false;

    unsigned int heart_beat_interval = config->GetIntValue(
                kConfigSection, "heart-beat-interval", kDefaultHeartBeatInterval);
    if (!heart_beat_scheduler_.Start<ContainerImpl, &ContainerImpl::send_heart_beat>(
                heart_beat_interval, this))
        return false;

    mlogger_.Info("Container initialized");
    return true;
}

void ContainerImpl::Finalize()
{
    mlogger_.Info("Finalizing Container ...");

    heart_beat_scheduler_.Stop();
    unregister_self();

    stop_services(false);
    unload_plugins();

    mlogger_.Info("Container finalized");
}

ResponseCode ContainerImpl::PrepareRequest(INetRequest *request)
{
    if (request == nullptr)
        return RC_BAD_REQUEST;

    const char* url = request->GetURL();
    const char* method = request->GetMethod();
    mlogger_.Info("Preparing request: %s %s", method, url);

    // TODO: PrepareHookedRequest

    agent_ptr_t agent = lookup_agent_by_url(url);
    if (agent.get() == nullptr)
    {
        mlogger_.Error("Service not found: %s %s", method, url);
        return RC_SERVICE_NOT_FOUND;
    }

    ResponseCode rc = agent->PrepareRequest(request);
    if (RC_OK != rc)
        mlogger_.Error("Failed to prepare request %s %s, rc = %d", method, url, rc);
    return rc;
}

ResponseCode ContainerImpl::ServeRequest(INetRequest *request, IOStream *ostream)
{
    if (request == nullptr || ostream == nullptr)
        return RC_BAD_REQUEST;

    const char* url = request->GetURL();
    const char* method = request->GetMethod();
    mlogger_.Info("Serving request: %s %s", method, url);

    // TODO: ServeHookedRequest

    agent_ptr_t agent = lookup_agent_by_url(url);
    if (agent.get() == nullptr)
    {
        mlogger_.Error("Service not found: %s %s", method, url);
        return RC_SERVICE_NOT_FOUND;
    }

    ResponseCode rc = agent->ServeRequest(request, ostream);
    mlogger_.Info("%s %s got rc = %d", method, url, rc);
    return rc;
}

bool ContainerImpl::load_plugins()
{
    const char* plugin_dir = config_->GetStrValue(kConfigSection, "plugin-dir", kDefaultPluginDirectory);
    common::ListDir dir(plugin_dir, common::ListDir::OMIT_DOT | common::ListDir::FILES);
    std::string fullname, shortname;
    const unsigned int extension_len = strlen(kPluginExtension);

    common::AutoLock autolock(plugins_lock_);

    while (dir.Next(&fullname, &shortname, nullptr))
    {
        if (shortname.size() < extension_len)
            continue;
        if (0 != shortname.compare(shortname.size() - extension_len, std::string::npos, kPluginExtension))
            continue;

        mlogger_.Info("Found plugin %s", shortname.c_str());

        PluginImpl *plugin = new PluginImpl(logger_, fullname.c_str());

        plugin_ref_t *plugin_ref = new plugin_ref_t();
        plugins_[shortname] = plugin_ref;

        plugin_ref->plugin_ = plugin;
        plugin_ref->fullname_ = fullname;
        plugin_ref->live_services_ = 0;

        const PluginImpl::service_creator_map_t &creators = plugin->DiscoverAvailableServices();
        if (creators.empty())
            mlogger_.Info("No service found in plugin %s: %s", shortname.c_str(), plugin->GetEmptyReason());
        else
        {
            for (PluginImpl::service_creator_map_t::const_iterator iter = creators.begin();
                iter != creators.end(); ++iter)
            {
                const std::string &service = iter->first;
                service::ServiceCreator creator = iter->second;

                plugin_ref->available_services_.insert(service);

                available_service_t &available_service = available_services_[service];
                available_service.plugin_ = shortname;
                available_service.creator_ = creator;
                mlogger_.Info("Service %s found", service.c_str());
            }
        }
    }
    mlogger_.Info("Totally %d plugins, %d services found", plugins_.size(), available_services_.size());

    return true;
}

bool ContainerImpl::determine_startup_services(std::vector<std::string> *startup_services)
{
    if (startup_services == nullptr)
        return false;

    const char* tmp = config_->GetStrValue(kConfigSection, "services", nullptr);
    if (tmp != nullptr)
    {
        std::istringstream iss(tmp);
        char buff[256];
        while (iss.getline(buff, sizeof(buff), ' '))
        {
            if (buff[0] == '\0')
                continue;
            startup_services->push_back(buff);
        }
    }

    mlogger_.Info("Totally %d startup services configured", startup_services->size());
    for (std::vector<std::string>::iterator iter = startup_services->begin();
        iter != startup_services->end(); ++iter)
    {
        mlogger_.Info("Startup service: %s", iter->c_str());
    }

    return true;
}

bool ContainerImpl::start_services(const std::vector<std::string> &startup_services)
{
    for (std::vector<std::string>::const_iterator iter = startup_services.begin();
        iter != startup_services.end(); ++iter)
    {
        start_service_asynchronously(*iter);
    }

    return true;
}

unsigned long ContainerImpl::stop_services(bool stop_dying_only)
{
    common::AutoWriteLock autolock(agents_lock_);

    unsigned int stopped = 0;
    for (agents_t::iterator iter = agents_.begin();
        iter != agents_.end(); )
    {
        const std::string &name = iter->first;
        ServiceAgentImpl *agent = iter->second.get();
        assert(agent != nullptr);

        if (stop_dying_only && agent->GetServiceState() != service::DYING)
        {
            ++iter;
            continue;
        }

        mlogger_.Info("Stopping service %s ...", name.c_str());
        agent->Finalize();
        mlogger_.Info("Service %s stopped", name.c_str());

        // erase from agents_ will cause the agent to be deleted through
        // agent_deletor_t, which will lock plugins_lock_, be aware
        // of the potential deadlock
        agents_.erase(iter++);
        ++stopped;
    }

    if (!stop_dying_only)
        assert(agents_.empty());

    return stopped;
}

void ContainerImpl::unload_plugins()
{
    common::AutoReadLock autolock1(agents_lock_);
    common::AutoLock autolock2(plugins_lock_);

    assert(agents_.empty());

    for (plugins_t::iterator iter = plugins_.begin();
        iter != plugins_.end(); ++iter)
    {
        plugin_ref_t *plugin_ref = iter->second;
        assert(plugin_ref != nullptr);
        assert(plugin_ref->live_services_ == 0);

        PluginImpl *plugin = plugin_ref->plugin_;
        assert(plugin != nullptr);

        delete plugin;
        delete plugin_ref;
    }

    plugins_.clear();
    available_services_.clear();
}

bool ContainerImpl::start_service_asynchronously(const std::string &service_name)
{
    common::AutoWriteLock autolock1(agents_lock_);
    common::AutoLock autolock2(plugins_lock_);

    mlogger_.Info("Creating service %s ...", service_name.c_str());
    if (agents_.find(service_name) != agents_.end())
    {
        mlogger_.Info("Service %s already exist", service_name.c_str());
        return false;
    }

    available_services_t::const_iterator iter1 = available_services_.find(service_name);
    if (iter1 == available_services_.end())
    {
        mlogger_.Error("Service %s unknown", service_name.c_str());
        return false;
    }

    service::ServiceCreator creator = iter1->second.creator_;
    assert(creator != nullptr);

    std::unique_ptr<ServiceAgentImpl> agent(new ServiceAgentImpl(env_, logger_, service_name.c_str()));
    service::IService *srv = creator(service_name.c_str(), agent.get(), env_, logger_);
    if (srv == nullptr)
    {
        mlogger_.Error("Failed to allocate/create service %s", service_name.c_str());
        return false;
    }

    if (!agent->InitializeAsynchronously(srv))
    {
        mlogger_.Error("Failed to start the initialization of service %s", service_name.c_str());
        return false;
    }

    const std::string &plugin_name = iter1->second.plugin_;
    plugins_t::iterator iter2 = plugins_.find(plugin_name);
    assert(iter2 != plugins_.end());
    assert(iter2->second != nullptr);
    ++(iter2->second->live_services_);

    agents_[service_name] = agent_ptr_t(agent.release(), agent_deletor_t(this, plugin_name));
    mlogger_.Info("Service %s created", service_name.c_str());
    return true;
}

void ContainerImpl::delete_agent(const std::string &plugin_name, ServiceAgentImpl *agent)
{
    common::AutoLock autolock(plugins_lock_);

    plugins_t::iterator iter = plugins_.find(plugin_name);
    assert(iter != plugins_.end());

    plugin_ref_t *plugin_ref = iter->second;
    assert(plugin_ref != nullptr);
    assert(plugin_ref->live_services_ > 0);
    --(plugin_ref->live_services_);

    delete agent;
}

ContainerImpl::agent_deletor_t::agent_deletor_t(ContainerImpl* container, const std::string &plugin) :
    container_(container),
    plugin_(plugin)
{
}

ContainerImpl::agent_deletor_t::agent_deletor_t(const agent_deletor_t &deletor) :
    container_(deletor.container_),
    plugin_(deletor.plugin_)
{
}

void ContainerImpl::agent_deletor_t::operator() (ServiceAgentImpl* agent)
{
    if (agent != nullptr && container_ != nullptr)
        container_->delete_agent(plugin_, agent);
}

ContainerImpl::agent_ptr_t ContainerImpl::lookup_agent_by_url(const char* url)
{
    if (url == nullptr || *url != '/')
        return agent_ptr_t();
    const char* second_slash = strchr(url+1, '/');
    if (second_slash == nullptr)
        return agent_ptr_t();

    std::string service_name(url+1, second_slash-url-1);
    common::AutoReadLock autolock(agents_lock_);
    agents_t::const_iterator iter = agents_.find(service_name);
    if (iter == agents_.end())
        return agent_ptr_t();

    return iter->second;
}

bool ContainerImpl::read_configuration()
{
    struct {
        const char* option_name;
        std::string *value;
    } values[] =
    {
        { "node-name", &my_node_name_        },
        { "host-name", &my_host_port_        },
        { "registry",  &registry_host_port_  },
    };

    for (unsigned int i=0; i<sizeof(values)/sizeof(values[0]); ++i)
    {
        const char* tmp = nullptr;
        if (nullptr == (tmp = config_->GetStrValue(kConfigSection, values[i].option_name, nullptr)))
        {
            mlogger_.Error("Config %s:%s not found!", kConfigSection, values[i].option_name);
            return false;
        }

        *(values[i].value) = tmp;
        mlogger_.Info("Config %s:%s = %s", kConfigSection, values[i].option_name, values[i].value->c_str());
    }

    return true;
}

void ContainerImpl::send_heart_beat()
{
    stop_services(true);

    mlogger_.Verbose("Sending heart-beat to %s ...", registry_host_port_.c_str());
    if (communicate_with_registry("/registry/heart-beat"))
        mlogger_.Info("Sending heart-beat to %s succeed", registry_host_port_.c_str());
    else
        mlogger_.Error("Sending heart-beat to %s failed", registry_host_port_.c_str());
}

void ContainerImpl::unregister_self()
{
    mlogger_.Verbose("Unregister this node \"%s\" from registry %s ...",
            my_node_name_.c_str(), registry_host_port_.c_str());

    if (communicate_with_registry("/registry/unregister"))
        mlogger_.Info("Unregister from %s succeed", registry_host_port_.c_str());
    else
        mlogger_.Error("Unregister from %s failed", registry_host_port_.c_str());
}

bool ContainerImpl::collect_node_info(service::registry::node_info_t *info)
{
    info->node_name = my_node_name_;
    info->host_port = my_host_port_;
    info->age = (runtime_env::ABSTime() - my_birth_time_).seconds();
    info->timeout = 0;

    {   common::AutoReadLock autolock(agents_lock_);
        info->services.reserve( agents_.size() );
        for (agents_t::const_iterator iter = agents_.begin();
            iter != agents_.end(); ++iter)
        {
            agent_ptr_t agent(iter->second);

            info->services.resize( info->services.size() + 1 );
            service::registry::service_info_t &service_info =
                info->services.back();
            service_info.service_name           = agent->GetServiceName();
            service_info.state                  = agent->GetServiceState();
            service_info.in_queue_size          = agent->GetInQueueSize();
            service_info.in_queue_doing_count   = agent->GetInQueueDoingCount();
            service_info.in_queue_traffic       = agent->GetInQueueTraffic();
            service_info.out_queue_size         = agent->GetOutQueueSize();
            service_info.out_queue_doing_count  = agent->GetOutQueueDoingCount();
            service_info.out_queue_traffic      = agent->GetOutQueueTraffic();
        }
    }

    {   common::AutoLock autolock(plugins_lock_);
        info->plugins.reserve( plugins_.size() );
        for (plugins_t::const_iterator iter = plugins_.begin();
            iter != plugins_.end(); ++iter)
        {
            info->plugins.resize( info->plugins.size() + 1 );
            service::registry::plugin_info_t &plugin_info =
                info->plugins.back();
            plugin_info.plugin_name = iter->first;

            std::vector<std::string> available_services(
                    iter->second->available_services_.begin(),
                    iter->second->available_services_.end());
            plugin_info.available_services.swap(available_services);
        }
    }

    return true;
}

bool ContainerImpl::communicate_with_registry(const char* uri)
{
    if (net_client_ == nullptr)
        return false;

    std::unique_ptr<INetClientConnection> conn(net_client_->NewConnection());
    if (conn.get() == nullptr)
        return false;

    service::registry::node_info_t node_info;
    if (!collect_node_info(&node_info))
        return false;

    if (!conn->Connect(registry_host_port_.c_str()))
        return false;

    IOStream* os = conn->StartRequest("POST", uri);
    if (os == nullptr)
        return false;
    if (!XsdSerialize(os, node_info))
        return false;
    if (!conn->EndRequest(&os))
        return false;

    int response_status = 0;
    IIStream *response_data = nullptr;
    unsigned long response_length = 0;

    if (!conn->Retrieve(&response_status, &response_data, &response_length))
        return false;
    if (response_status != 200)
        return false;

    std::unique_ptr<service::registry::nodes_info_t> nodes_info(new service::registry::nodes_info_t());
    if (!XsdUnserialize(response_data, *nodes_info))
        return false;

    nodes_info_.reset(nodes_info.release());

    conn->Close(&response_data);
    return true;
}

}}  // namespace fasmio::container

