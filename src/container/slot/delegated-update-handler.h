
#ifndef CONTAINER_SLOT_DELEGATED_UPDATE_HANDLER_H_
#define CONTAINER_SLOT_DELEGATED_UPDATE_HANDLER_H_

#include "./slot-handler.h"

namespace fasmio { namespace container {

class DelegatedUpdateHandler : public SlotHandler
{
public:
    typedef bool (ServiceAgentImpl::*Handler)(service::IRequest*, service::ITask**);

public:
    DelegatedUpdateHandler(ServiceAgentImpl *agent, Handler);
    virtual ~DelegatedUpdateHandler();

public:
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*);

private:
    Handler handler_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_SLOT_DELEGATED_UPDATE_HANDLER_H_

