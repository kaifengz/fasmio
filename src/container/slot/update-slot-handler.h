
#ifndef CONTAINER_SLOT_UPDATE_SLOT_HANDLER_H_
#define CONTAINER_SLOT_UPDATE_SLOT_HANDLER_H_

#include "./slot-handler.h"

namespace fasmio { namespace container {

class UpdateSlotHandler : public SlotHandler
{
public:
    UpdateSlotHandler(ServiceAgentImpl *agent, const char* slot_name, service::TaskAllocator alloc, service::UpdateSlotHandler handler);
    virtual ~UpdateSlotHandler();

public:
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*);

private:
    service::TaskAllocator allocator_;
    service::UpdateSlotHandler handler_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_SLOT_UPDATE_SLOT_HANDLER_H_

