
#include "./update-slot-handler.h"
#include <assert.h>

namespace fasmio { namespace container {

UpdateSlotHandler::UpdateSlotHandler(
            ServiceAgentImpl *agent,
            const char* slot_name,
            service::TaskAllocator alloc,
            service::UpdateSlotHandler handler) :
    SlotHandler(agent, slot_name),
    allocator_(alloc),
    handler_(handler)
{
}

UpdateSlotHandler::~UpdateSlotHandler()
{
}

ResponseCode UpdateSlotHandler::ServeRequest(INetRequest *request, IOStream *ostream)
{
    assert(request != nullptr && ostream != nullptr);

    service::IService *srv = agent_->GetService();
    std::unique_ptr<service::ITask> in_task(allocator_(srv));
    if (!ParseFromRequest(request, in_task.get()))
        return RC_BAD_REQUEST;

    std::unique_ptr<service::IRequest> req(MakeRequest(request));
    service::ITask *out_task = nullptr;
    if (!handler_(agent_->GetService(), req.get(), in_task.get(), &out_task))
        return RC_BAD_REQUEST;

    if (out_task != nullptr)
    {
        std::unique_ptr<service::ITask> task(out_task);
        if (task->AnteSerialize() && task->Serialize(ostream))
            return RC_OK;
        else
            return RC_SERVICE_ERROR;
    }

    return RC_OK;
}

}}  // namespace fasmio::container

