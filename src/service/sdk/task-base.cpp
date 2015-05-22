
#include "./task-base.h"
#include "common/stream-adaptor.h"

namespace fasmio { namespace service { namespace sdk {

TaskBase::TaskBase() :
    meta_(nullptr)
{
}

TaskBase::~TaskBase()
{
    if (meta_ != nullptr)
        delete meta_;
}

ITaskMeta* TaskBase::GetTaskMeta()
{
    return meta_;
}

bool TaskBase::SetTaskMeta(ITaskMeta *meta)
{
    if (meta_ != nullptr)
        delete meta_;
    meta_ = meta;
    return true;
}

bool TaskBase::Serialize(IOStream *os)
{
    common::OStreamAdaptor adaptor(os);
    return Serialize(adaptor);
}

bool TaskBase::UnSerialize(IIStream *is)
{
    common::IStreamAdaptor adaptor(is);
    return UnSerialize(adaptor);
}

bool TaskBase::AnteSerialize()
{
    return true;
}

bool TaskBase::PostUnSerialize()
{
    return true;
}

}}}  // namespace fasmio::service::sdk

