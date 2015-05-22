
#ifndef CONTAINER_SLOT_DELEGATED_QUERY_HANDLER_H_
#define CONTAINER_SLOT_DELEGATED_QUERY_HANDLER_H_

#include "./slot-handler.h"

namespace fasmio { namespace container {

class DelegatedQueryHandler : public SlotHandler
{
public:
    DelegatedQueryHandler(ServiceAgentImpl *agent, const char* slot_name);
    virtual ~DelegatedQueryHandler();

public:
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*);

private:
};

}}  // namespace fasmio::container

#endif  // CONTAINER_SLOT_DELEGATED_QUERY_HANDLER_H_

