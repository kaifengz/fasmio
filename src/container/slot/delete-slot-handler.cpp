
#include "./delete-slot-handler.h"
#include <assert.h>

namespace fasmio { namespace container {

DeleteSlotHandler::DeleteSlotHandler(ServiceAgentImpl *agent, const char* slot_name, service::DeleteSlotHandler handler) :
    SlotHandler(agent, slot_name),
    handler_(handler)
{
}

DeleteSlotHandler::~DeleteSlotHandler()
{
}

ResponseCode DeleteSlotHandler::ServeRequest(INetRequest *request, IOStream *ostream)
{
    assert(request != nullptr && ostream != nullptr);

    std::unique_ptr<service::IRequest> req(MakeRequest(request));
    if (!handler_(agent_->GetService(), req.get()))
        return RC_BAD_REQUEST;

    return RC_OK;
}

}}  // namespace fasmio::container

