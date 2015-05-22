
#ifndef CONTAINER_SLOT_DELETE_SLOT_HANDLER_H_
#define CONTAINER_SLOT_DELETE_SLOT_HANDLER_H_

#include "./slot-handler.h"

namespace fasmio { namespace container {

class DeleteSlotHandler : public SlotHandler
{
public:
    DeleteSlotHandler(ServiceAgentImpl *agent, const char* slot_name, service::DeleteSlotHandler handler);
    virtual ~DeleteSlotHandler();

public:
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*);

private:
    service::DeleteSlotHandler handler_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_SLOT_DELETE_SLOT_HANDLER_H_

