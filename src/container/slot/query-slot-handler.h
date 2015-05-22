
#ifndef CONTAINER_SLOT_QUERY_SLOT_HANDLER_H_
#define CONTAINER_SLOT_QUERY_SLOT_HANDLER_H_

#include "./slot-handler.h"

namespace fasmio { namespace container {

class QuerySlotHandler : public SlotHandler
{
public:
    QuerySlotHandler(ServiceAgentImpl *agent, const char* slot_name, service::QuerySlotHandler handler);
    virtual ~QuerySlotHandler();

public:
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*);

private:
    service::QuerySlotHandler handler_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_SLOT_QUERY_SLOT_HANDLER_H_

