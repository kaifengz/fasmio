
#include "./delegated-query-handler.h"
#include <assert.h>

namespace fasmio { namespace container {

DelegatedQueryHandler::DelegatedQueryHandler(ServiceAgentImpl *agent, const char* slot_name) :
    SlotHandler(agent, slot_name)
{
}

DelegatedQueryHandler::~DelegatedQueryHandler()
{
}

ResponseCode DelegatedQueryHandler::ServeRequest(INetRequest *request, IOStream *ostream)
{
    assert(request != nullptr && ostream != nullptr);

    // TODO: DelegatedQueryHandler::ServeRequest
    return RC_OK;
}

}}  // namespace fasmio::container

