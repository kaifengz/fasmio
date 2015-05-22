
#include "./query-slot-handler.h"
#include <assert.h>

namespace fasmio { namespace container {

QuerySlotHandler::QuerySlotHandler(ServiceAgentImpl *agent, const char* slot_name, service::QuerySlotHandler handler) :
    SlotHandler(agent, slot_name),
    handler_(handler)
{
}

QuerySlotHandler::~QuerySlotHandler()
{
}

ResponseCode QuerySlotHandler::ServeRequest(INetRequest *request, IOStream *ostream)
{
    assert(request != nullptr && ostream != nullptr);

    std::unique_ptr<service::IRequest> req(MakeRequest(request));
    service::ITask *out_task = nullptr;
    if (!handler_(agent_->GetService(), req.get(), &out_task))
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

