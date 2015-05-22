
#include "./slot-handler.h"
#include "../request-impl.h"
#include <assert.h>

namespace fasmio { namespace container {

SlotHandler::SlotHandler(ServiceAgentImpl *agent, const char* slot_name) :
    agent_(agent),
    slot_name_(slot_name)
{
}

SlotHandler::~SlotHandler()
{
}

service::IRequest* SlotHandler::MakeRequest(INetRequest *req)
{
    std::unique_ptr<RequestImpl> request(new RequestImpl());
    if (!request->BuildFrom(req))
        return nullptr;

    return static_cast<service::IRequest*>(request.release());
}

bool SlotHandler::ParseFromRequest(INetRequest *request, service::ITask *task)
{
    assert(request != nullptr && task != nullptr);

    IIStream* is = request->GetContent();
    if (is == nullptr)
        return false;

    if (!task->UnSerialize(is))
        return false;

    return task->PostUnSerialize();
}

}}  // namespace fasmio::container

