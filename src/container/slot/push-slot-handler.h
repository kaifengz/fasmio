
#ifndef CONTAINER_SLOT_PUSH_SLOT_HANDLER_H_
#define CONTAINER_SLOT_PUSH_SLOT_HANDLER_H_

#include "./slot-handler.h"
#include "../tlv-parser.h"

namespace fasmio { namespace container {

class PushSlotHandler : public SlotHandler
{
public:
    PushSlotHandler(ServiceAgentImpl *agent, const char* slot_name, service::TaskAllocator alloc, std::shared_ptr<QueueImpl> queue);
    virtual ~PushSlotHandler();

public:
    virtual ResponseCode ServeRequest(INetRequest*, IOStream*);

private:
    struct ParseContext
    {
        PushSlotHandler* handler;
        TlvParser* parser;
        enum {
            NONE = 0,
            ROOT,
            META,
        } state;
        std::unique_ptr<service::ITask> task;
        unsigned long count;
    };

private:
    static bool OnEnterKey(const char* name, void* arg);
    static bool OnLeaveKey(const char* name, void* arg);
    static bool OnValue   (const char* name, IIStream *, void* arg);

private:
    service::TaskAllocator allocator_;
    std::shared_ptr<QueueImpl> queue_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_SLOT_PUSH_SLOT_HANDLER_H_

