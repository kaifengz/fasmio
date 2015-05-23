
#include "./task-meta-impl.h"
#include "common/stream-adaptor.h"
#include "common/uuid.h"
#include <assert.h>
#include <string.h>

#define DEFAULT_META_FORMAT fasmio::service::TMF_TLV

namespace fasmio { namespace container {

TaskMetaImpl::TaskMetaImpl() :
    task_ids_(),
    generator_node_(),
    generator_service_(),
    birth_time_(0.0),
    need_ack_(true),
    resend_count_(0),
    resend_time_(0.0),
    user_data_()
{
}

TaskMetaImpl::TaskMetaImpl(const char* task_id) :
    task_ids_(),
    generator_node_(),
    generator_service_(),
    birth_time_(0.0),
    need_ack_(true),
    resend_count_(0),
    resend_time_(0.0),
    user_data_()
{
    assert(task_id != nullptr);
    task_ids_.push_back(task_id);
}

TaskMetaImpl::TaskMetaImpl(const TaskMetaImpl &meta) :
    task_ids_(meta.task_ids_),
    generator_node_(meta.generator_node_),
    generator_service_(meta.generator_service_),
    birth_time_(meta.birth_time_),
    need_ack_(meta.need_ack_),
    resend_count_(meta.resend_count_),
    resend_time_(meta.resend_time_),
    user_data_(meta.user_data_.get() != nullptr ? meta.user_data_->Clone() : nullptr)
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
    return generator_node_.c_str();
}

const char* TaskMetaImpl::GetGeneratorService()
{
    return generator_service_.c_str();
}

double TaskMetaImpl::GetBirthTime()
{
    return birth_time_;
}

bool TaskMetaImpl::NeedAck()
{
    return need_ack_;
}

unsigned long TaskMetaImpl::GetResendCount()
{
    return resend_count_;
}

double TaskMetaImpl::GetResendTime()
{
    return resend_time_;
}

bool TaskMetaImpl::SetGeneratorInfo(const char* node,
                                    const char* service,
                                    double birth_time,
                                    bool need_ack)
{
    if (node == nullptr || service == nullptr)
        return false;

    generator_node_ = node;
    generator_service_ = service;
    birth_time_ = birth_time;
    need_ack_ = need_ack;
    return true;
}

bool TaskMetaImpl::SetResendInfo(unsigned long resend_count,
                                 double resend_time)
{
    resend_count_ = resend_count;
    resend_time_ = resend_time;
    return true;
}

service::ITaskJourney* TaskMetaImpl::GetJourney()
{
    // TODO:
    return nullptr;
}

service::IKey* TaskMetaImpl::GetUserData()
{
    if (user_data_.get() == nullptr)
        user_data_.reset(new KeyValueImpl("User", KeyValue_Key));

    return static_cast<service::IKey*>(user_data_.get());
}

bool TaskMetaImpl::ClearJourney()
{
    // TODO:
    return true;
}

bool TaskMetaImpl::ClearUserData()
{
    if (user_data_.get() != nullptr)
        user_data_->Clear();

    return true;
}

service::ITaskMeta* TaskMetaImpl::Clone()
{
    return new TaskMetaImpl(*this);
}

service::ITaskMeta* TaskMetaImpl::Fork()
{
    TaskMetaImpl *meta = new TaskMetaImpl(*this);
    if (meta != nullptr)
        meta->task_ids_.push_back(common::GenUUID());
    return meta;
}

service::ITaskMeta* TaskMetaImpl::Fork(const char* task_id)
{
    if (task_id == nullptr)
        return nullptr;

    TaskMetaImpl *meta = new TaskMetaImpl(*this);
    if (meta != nullptr)
        meta->task_ids_.push_back(task_id);
    return meta;
}

bool TaskMetaImpl::Serialize(IOStream* os)
{
    return Serialize(os, DEFAULT_META_FORMAT);
}

bool TaskMetaImpl::Serialize(IOStream* os, service::TaskMetaFormat format)
{
    switch (format)
    {
    case fasmio::service::TMF_TLV:
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
    TlvParser parser(is);
    return TlvParser::R_KEY == UnSerialize(&parser);
}

bool TaskMetaImpl::Serialize(TlvComposer *composer)
{
    if (composer == nullptr)
        return false;

    if (!task_ids_.empty())
    {
        if (!composer->BeginKey("TaskIds"))
            return false;
        for (std::vector<std::string>::const_iterator iter = task_ids_.begin();
            iter != task_ids_.end(); ++iter)
        {
            if (!composer->AddValue("TaskId", iter->c_str()))
                return false;
        }
        if (!composer->EndKey("TaskIds"))
            return false;
    }

    if (!composer->BeginKey("Generator") ||
        !composer->AddValue("Node", generator_node_.c_str()) ||
        !composer->AddValue("Service", generator_service_.c_str()) ||
        !composer->AddValue("BirthTime", birth_time_) ||
        (!need_ack_ && !composer->AddValue("NeedAck", need_ack_)) ||
        !composer->AddValue("ResendCount", resend_count_) ||
        (resend_count_ > 0 && !composer->AddValue("ResendTime", resend_time_)) ||
        !composer->EndKey("Generator"))
    {
        return false;
    }

    // TODO: Serialize Journey

    if (user_data_.get() != nullptr && user_data_->ChildrenCount() > 0)
    {
        KeyValueImpl *user = static_cast<KeyValueImpl*>(user_data_.get());
        if (!user->Serialize(composer))
            return false;
    }

    return true;
}

TlvParser::ParseResult TaskMetaImpl::UnSerialize(TlvParser *parser)
{
    if (parser == nullptr)
        return TlvParser::R_FAILED;

    KeyValueImpl meta_kv;
    TlvParser::ParseResult result = meta_kv.UnSerialize(parser);
    if (result != TlvParser::R_KEY)
        return result;

    TaskMetaImpl meta;
    if (!meta.parse_from(static_cast<service::IKey*>(&meta_kv)))
        return TlvParser::R_FAILED;

    this->Swap(meta);
    return result;
}

void TaskMetaImpl::Swap(TaskMetaImpl &meta)
{
    std::swap(task_ids_,           meta.task_ids_);
    std::swap(generator_node_,     meta.generator_node_);
    std::swap(generator_service_,  meta.generator_service_);
    std::swap(birth_time_,         meta.birth_time_);
    std::swap(need_ack_,           meta.need_ack_);
    std::swap(resend_count_,       meta.resend_count_);
    std::swap(resend_time_,        meta.resend_time_);
    std::swap(user_data_,          meta.user_data_);
}

void TaskMetaImpl::allocate_id()
{
    assert(task_ids_.empty());
    task_ids_.push_back(common::GenUUID());
}

bool TaskMetaImpl::parse_from(service::IKey* meta)
{
    service::IKey *key = nullptr;

    if (nullptr == (key = meta->GetKey("Generator")))
        // generator info must included
        return false;
    else
    {
        const char* text_value = nullptr;
        if (nullptr == (text_value = key->GetTextValue("Node")))
            return false;
        generator_node_ = text_value;
        if (nullptr == (text_value = key->GetTextValue("Service")))
            return false;
        generator_service_ = text_value;
        if (!key->TryGetDoubleValue("BirthTime", &birth_time_))
            return false;
        key->TryGetBoolValue("NeedAck", &need_ack_);
        if (!key->TryGetLongValue("ResendCount", &resend_count_))
            return false;
        if (resend_count_ > 0 && !key->TryGetDoubleValue("ResendTime", &resend_time_))
            return false;
    }

    if (nullptr != (key = meta->GetKey("TaskIds")))
    {
        for (service::IValue *value = key->GetFirstValue(); value != nullptr; value = value->GetNextValue())
        {
            if (0 == strcasecmp("TaskId", value->GetName()))
                task_ids_.push_back(value->GetTextValue());
        }
    }

    if (nullptr != (key = meta->GetKey("Journey")))
    {
        // TODO:
    }

    user_data_.reset(meta->ReleaseKey("User"));
    return true;
}

}}  // namespace fasmio::container

