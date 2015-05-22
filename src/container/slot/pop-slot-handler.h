
#ifndef CONTAINER_SLOT_POP_SLOT_HANDLER_H_
#define CONTAINER_SLOT_POP_SLOT_HANDLER_H_

#include "./slot-handler.h"
#include "../data-types.xsd.h"

namespace fasmio { namespace container {

class PopSlotHandler : public SlotHandler
{
public:
    PopSlotHandler(ServiceAgentImpl *agent, const char* slot_name, std::shared_ptr<QueueImpl> queue);
    virtual ~PopSlotHandler();

public:
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*);

private:
    bool SerializeTasks(
            unsigned int count, service::ITask* tasks[], unsigned int size_limit,
            IOStream *ostream, unsigned int *serialized);

private:
    std::shared_ptr<QueueImpl> queue_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_SLOT_POP_SLOT_HANDLER_H_

