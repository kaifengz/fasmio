
#include "./task-meta-impl.h"
#include "common/stream-adaptor.h"
#include <assert.h>

#define DEFAULT_META_FORMAT fasmio::service::TMF_BINARY

namespace fasmio { namespace container {

TaskMetaImpl::TaskMetaImpl() :
    task_ids_()
{
}

TaskMetaImpl::~TaskMetaImpl()
{
}

const char* TaskMetaImpl::GetID()
{
    if (task_ids_.empty())
        allocate_id();

    return task_ids_.back().c_str();
}

const char* TaskMetaImpl::GetRootID()
{
    if (task_ids_.empty())
        allocate_id();

    return task_ids_.front().c_str();
}

const char* TaskMetaImpl::GetParentID()
{
    if (task_ids_.empty())
        allocate_id();

    if (task_ids_.size() == 1)
        return task_ids_.back().c_str();

    std::vector<std::string>::reverse_iterator iter = task_ids_.rbegin();
    return (++iter)->c_str();
}

const char* TaskMetaImpl::GetParentID(const char* id)
{
    if (id == nullptr)
        return nullptr;

    if (task_ids_.empty())
        allocate_id();

    for (std::vector<std::string>::reverse_iterator iter = task_ids_.rbegin();
        iter != task_ids_.rend(); ++iter)
    {
        if (*iter == id)
        {
            if (++iter != task_ids_.rend())
                return iter->c_str();
            else
                return nullptr;
        }
    }

    return nullptr;
}

const char* TaskMetaImpl::GetGeneratorNode()
{
    // TODO:
    return nullptr;
}

const char* TaskMetaImpl::GetGeneratorService()
{
    // TODO:
    return nullptr;
}

double TaskMetaImpl::GetBirthTime()
{
    // TODO:
    return 0.0;
}

bool TaskMetaImpl::NeedAck()
{
    // TODO:
    return true;
}

unsigned int TaskMetaImpl::GetResendCount()
{
    // TODO:
    return 0;
}

double TaskMetaImpl::GetResendTime()
{
    // TODO:
    return 0.0;
}

bool TaskMetaImpl::SetGeneratorInfo(const char* node,
                                    const char* service,
                                    double birth_time,
                                    bool need_ack)
{
    // TODO:
    return true;
}

bool TaskMetaImpl::SetResendInfo(unsigned int resend_count,
                                    double resend_time)
{
    // TODO:
    return true;
}

service::ITaskJourney* TaskMetaImpl::GetJourney()
{
    // TODO:
    return nullptr;
}

service::IKey* TaskMetaImpl::GetUserData()
{
    // TODO:
    return nullptr;
}

bool TaskMetaImpl::ClearJourney()
{
    // TODO:
    return true;
}

bool TaskMetaImpl::ClearUserData()
{
    // TODO:
    return true;
}

service::ITaskMeta* TaskMetaImpl::Clone()
{
    // TODO:
    return nullptr;
}

service::ITaskMeta* TaskMetaImpl::Fork()
{
    // TODO:
    return nullptr;
}

service::ITaskMeta* TaskMetaImpl::Fork(const char*)
{
    // TODO:
    return nullptr;
}

bool TaskMetaImpl::Serialize(IOStream* os)
{
    return Serialize(os, DEFAULT_META_FORMAT);
}

bool TaskMetaImpl::Serialize(IOStream* os, service::TaskMetaFormat format)
{
    switch (format)
    {
    case fasmio::service::TMF_BINARY:
        {
            TlvComposer composer(os);
            return Serialize(&composer) && composer.CheckIntegrity();
        }

    case fasmio::service::TMF_XML:
        // TODO:
        return false;

    default:
        return false;
    }
}

bool TaskMetaImpl::UnSerialize(IIStream* is)
{
//    common::IStreamAdaptor adaptor(is);
//    return UnSerialize(adaptor);
    return false;
}

bool TaskMetaImpl::Serialize(TlvComposer *composer)
{
    if (composer == nullptr)
        return false;

    // TODO:
    return true;
}

bool TaskMetaImpl::UnSerialize(TlvParser *parser)
{
    if (parser == nullptr)
        return false;

    parser->PushCallbacks();

    // TODO:

    parser->RestoreCallbacks();
    return true;
}

void TaskMetaImpl::allocate_id()
{
    assert(task_ids_.empty());
    task_ids_.push_back("TODO: random UUID as task id"); // TODO
}

}}  // namespace fasmio::container

