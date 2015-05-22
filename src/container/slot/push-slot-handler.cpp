
#include "./push-slot-handler.h"
#include "../task-meta-impl.h"
#include "common/stream-adaptor.h"

#include <assert.h>
#include <string.h>

namespace fasmio { namespace container {

PushSlotHandler::PushSlotHandler(
            ServiceAgentImpl *agent,
            const char* slot_name,
            service::TaskAllocator alloc,
            std::shared_ptr<QueueImpl> queue) :
    SlotHandler(agent, slot_name),
    allocator_(alloc),
    queue_(queue)
{
}

PushSlotHandler::~PushSlotHandler()
{
}

ResponseCode PushSlotHandler::ServeRequest(INetRequest *request, IOStream *ostream)
{
    assert(request != nullptr && ostream != nullptr);

    IIStream *is = request->GetContent();
    if (is == nullptr)
        return RC_BAD_REQUEST;

    ParseContext context;

    TlvParser parser(is);
    parser.SetEnterKeyCallback(OnEnterKey, &context);
    parser.SetLeaveKeyCallback(OnLeaveKey, &context);
    parser.SetValueCallback(OnValue, &context);

    context.handler = this;
    context.parser = &parser;
    context.state = ParseContext::NONE;
    context.count = 0;

    bool succeed = parser.Parse();

    agent_->Info("%d tasks pushed to /%s/%s", context.count, agent_->GetServiceName(), slot_name_);
    return succeed ? RC_OK : RC_BAD_REQUEST;
}

bool PushSlotHandler::OnEnterKey(const char* name, void* arg)
{
    ParseContext *context = reinterpret_cast<ParseContext*>(arg);
    assert(context != nullptr);

    if (context->state != ParseContext::NONE)
        return false;

    if (0 == strcasecmp(name, "root"))
    {
        context->state = ParseContext::ROOT;
        return true;
    }
    else if (0 == strcasecmp(name, "meta"))
    {
        std::unique_ptr<TaskMetaImpl> meta(new TaskMetaImpl);
        if (!meta->UnSerialize(context->parser))
            return false;

        assert(context->task.get() == nullptr);
        context->task.reset(context->handler->allocator_(context->handler->agent_->GetService()));
        context->task->SetTaskMeta(meta.release());
        context->state = ParseContext::META;
        return true;
    }
    else
    {
        return false;
    }
}

bool PushSlotHandler::OnLeaveKey(const char* name, void* arg)
{
    ParseContext *context = reinterpret_cast<ParseContext*>(arg);
    assert(context != nullptr);

    if (context->state != ParseContext::ROOT)
        return false;

    context->state = ParseContext::NONE;
    return true;
}

bool PushSlotHandler::OnValue(const char* name, IIStream *is, void* arg)
{
    ParseContext *context = reinterpret_cast<ParseContext*>(arg);
    assert(context != nullptr);

    PushSlotHandler *handler = context->handler;
    assert(handler != nullptr);

    switch (context->state)
    {
//    case ParseContext::ROOT:
//        {
//            if (0 != strcasecmp(name, "meta"))
//                return false;
//
//            std::unique_ptr<TaskMetaImpl> meta(new TaskMetaImpl);
//            if (!meta->UnSerialize(is))
//                return false;
//
//            assert(context->task.get() == nullptr);
//            context->task.reset(handler->allocator_(handler->agent_->GetService()));
//            context->task->SetTaskMeta(meta.release());
//            context->state = ParseContext::META;
//            return true;
//        }

    case ParseContext::META:
        {
            if (0 != strcasecmp(name, "task"))
                return false;

            assert(context->task.get() != nullptr);
            assert(context->task->GetTaskMeta() != nullptr);
            if (!context->task->UnSerialize(is))
                return false;

            handler->queue_->AddTask(context->task.release());
            context->state = ParseContext::ROOT;
            ++(context->count);
            return true;
        }

    default:
        return false;
    }
}

}}  // namespace fasmio::container

