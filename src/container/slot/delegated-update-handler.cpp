
#include "./delegated-update-handler.h"
#include <assert.h>

namespace fasmio { namespace container {

DelegatedUpdateHandler::DelegatedUpdateHandler(ServiceAgentImpl *agent, Handler handler) :
    SlotHandler(agent, nullptr),
    handler_(handler)
{
}

DelegatedUpdateHandler::~DelegatedUpdateHandler()
{
}

ResponseCode DelegatedUpdateHandler::ServeRequest(INetRequest *request, IOStream *ostream)
{
    assert(request != nullptr && ostream != nullptr);

    std::unique_ptr<service::IRequest> req(MakeRequest(request));
    service::ITask *out_task = nullptr;
    if (!(agent_->*handler_)(req.get(), &out_task))
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

