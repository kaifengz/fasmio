
#include "./registry.h"
#include "interface/runtime-env/rwlock.h"
#include "common/auto-lock.h"

namespace fasmio { namespace service { namespace registry {

REGISTER_SERVICE("registry", Registry);

const unsigned long kTimeoutThreshold = 30;

Registry::Registry(const char* name, IServiceAgent *agent, IRuntimeEnv* env, ILogger *logger) :
    ServiceBase(name, agent, env, logger),
    nodes_(),
    nodes_lock_(env->NewRWLock())
{
}

bool Registry::OnInitialize()
{
    if (!RegisterUpdateSlot<Registry, NodeTask, NodesTask, &Registry::DoHeartBeat>("heart-beat", NodeTask::Allocator))
        return false;
    if (!RegisterUpdateSlot<Registry, NodeTask, NodesTask, &Registry::DoUnregister>("unregister", NodeTask::Allocator))
        return false;
    if (!RegisterQuerySlot<Registry, NodesTask, &Registry::DoGetNodes>("nodes"))
        return false;
    if (!RegisterTimer<Registry, &Registry::MaintainNodes>("reg-nodes-maint", 1000))
        return false;

    return true;
}

void Registry::OnFinalize()
{
}

bool Registry::DoHeartBeat(IRequest*, NodeTask *node_info, NodesTask **nodes_info)
{
    return heart_beat(node_info) && get_all_nodes(nodes_info);
}

bool Registry::DoUnregister(IRequest*, NodeTask *node_info, NodesTask **nodes_info)
{
    return unregister(node_info) && get_all_nodes(nodes_info);
}

bool Registry::DoGetNodes(IRequest*, NodesTask** nodes_info)
{
    return get_all_nodes(nodes_info);
}

void Registry::MaintainNodes()
{
    common::AutoWriteLock autolock(nodes_lock_);
    for (nodes_t::iterator iter = nodes_.begin();
        iter != nodes_.end();
        ++iter)
    {
        node_info_t &node = iter->second;
        if (++node.timeout >= kTimeoutThreshold)
        {
            Error("Drop node %s at %s as its heart-beat not received for %d seconds",
                node.node_name.c_str(), node.host_port.c_str(), node.timeout);
            nodes_.erase(iter++);
        }
    }
}

bool Registry::heart_beat(NodeTask *node_info)
{
    if (node_info == nullptr)
        return false;

    node_info_t &node = **node_info;
    if (node.node_name.empty() || node.host_port.empty())
    {
        Error("Heart-beat request invalid: node-name = \"%s\", host-port = \"%s\"",
            node.node_name.c_str(),
            node.host_port.c_str());
        return false;
    }

    node.timeout = 0;

    common::AutoWriteLock autolock(nodes_lock_);
    nodes_t::iterator iter = nodes_.find(node.node_name);
    if (iter != nodes_.end())
    {
        Info("Heart-beat from %s at %s",
                node.node_name.c_str(),
                node.host_port.c_str());

        // node already exist in registry
        node_info_t &existent = iter->second;
        if (existent.host_port != node.host_port)
        {
            Error("Node %s relocated from %s to %s",
                    node.node_name.c_str(),
                    existent.host_port.c_str(),
                    node.host_port.c_str());
        }
        existent = node;
    }
    else
    {
        Info("Heart-beat from %s at %s (new node)",
                node.node_name.c_str(),
                node.host_port.c_str());

        nodes_[node.node_name] = node;
    }

    return true;
}

bool Registry::unregister(NodeTask *node_info)
{
    if (node_info == nullptr)
        return false;
    node_info_t &node = **node_info;

    common::AutoWriteLock autolock(nodes_lock_);
    nodes_t::iterator iter = nodes_.find(node.node_name);
    if (iter != nodes_.end())
    {
        Info("Node %s at %s unregisters itself from registry",
                node.node_name.c_str(),
                node.host_port.c_str());
        nodes_.erase(iter);
    }
    else
    {
        Info("Node %s at %s tries to unregister itself, which is indeed not in registry",
                node.node_name.c_str(),
                node.host_port.c_str());
    }

    return true;
}

bool Registry::get_all_nodes(NodesTask **nodes_info)
{
    if (nodes_info == nullptr)
        return false;

    common::AutoReadLock autolock(nodes_lock_);

    std::unique_ptr<NodesTask> nodes_task(new NodesTask);
    nodes_info_t &nodes = **nodes_task;
    nodes.reserve( nodes_.size() );

    for (nodes_t::const_iterator iter = nodes_.begin();
        iter != nodes_.end(); ++iter)
    {
        nodes.push_back(iter->second);
    }

    *nodes_info = nodes_task.release();
    return true;
}

}}}  // namespace fasmio::service::registry

