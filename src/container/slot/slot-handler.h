
#ifndef CONTAINER_SLOT_SLOT_HANDLER_H_
#define CONTAINER_SLOT_SLOT_HANDLER_H_

#include "interface/container.h"
#include "service/interface/request.h"
#include "service/interface/task.h"
#include "../service-agent-impl.h"
#include <assert.h>

namespace fasmio { namespace container {

class SlotHandler
{
public:
    SlotHandler(ServiceAgentImpl *agent, const char* slot_name);
    virtual ~SlotHandler();

public:
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*) = 0;

protected:
    service::IRequest* MakeRequest(INetRequest*);

    template <typename XsdType>
    bool ParseFromRequest(INetRequest *request, XsdType *data)
    {
        assert(request != nullptr && data != nullptr);

        IIStream* is = request->GetContent();
        return is != nullptr && XsdUnserialize(is, *data);
    }

    bool ParseFromRequest(INetRequest*, service::ITask*);

protected:
    ServiceAgentImpl * const agent_;
    const char* const slot_name_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_SLOT_SLOT_HANDLER_H_

