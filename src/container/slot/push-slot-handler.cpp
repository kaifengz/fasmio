
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

#define INFO    context->handler->agent_->Info

bool PushSlotHandler::OnEnterKey(const char* name, void* arg)
{
    ParseContext *context = reinterpret_cast<ParseContext*>(arg);
    assert(context != nullptr);

    if (0 != strcasecmp(name, "root"))
    {
        INFO("Unexpected top level key '%s'", name);
        return false;
    }

    while (true)
    {
        TlvParser::ParseResult result;

        std::unique_ptr<TaskMetaImpl> meta(new TaskMetaImpl);
        result = meta->UnSerialize(context->parser);
        if (result == TlvParser::R_KEY_END)
            break;

        if (result != TlvParser::R_KEY)
        {
            INFO("Failed to unserialize the META");
            return false;
        }

        assert(context->task.get() == nullptr);
        context->task.reset(context->handler->allocator_(context->handler->agent_->GetService()));
        context->task->SetTaskMeta(meta.release());

        if (TlvParser::R_VALUE != (result = context->parser->Parse()))
            return false;
    }

    return true;
}

bool PushSlotHandler::OnLeaveKey(const char* name, void* arg)
{
    return false;
}

bool PushSlotHandler::OnValue(const char* name, IIStream *is, void* arg)
{
    ParseContext *context = reinterpret_cast<ParseContext*>(arg);
    assert(context != nullptr);
    assert(context->handler != nullptr);
    assert(context->handler->queue_ != nullptr);

    if (0 != strcasecmp(name, "task"))
        return false;

    assert(context->task.get() != nullptr);
    assert(context->task->GetTaskMeta() != nullptr);
    if (!context->task->UnSerialize(is))
    {
        INFO("Failed to unserialize the TASK");
        return false;
    }

    context->handler->queue_->AddTask(context->task.release());
    ++(context->count);
    return true;
}

}}  // namespace fasmio::container

