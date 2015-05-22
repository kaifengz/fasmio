
#include "./pop-slot-handler.h"
#include "../task-meta-impl.h"
#include "../stream-impl.h"
#include "../tlv-composer.h"
#include "common/stream-adaptor.h"
#include <memory.h>

namespace fasmio { namespace container {

PopSlotHandler::PopSlotHandler(
            ServiceAgentImpl *agent,
            const char* slot_name,
            std::shared_ptr<QueueImpl> queue) :
    SlotHandler(agent, slot_name),
    queue_(queue)
{
}

PopSlotHandler::~PopSlotHandler()
{
}

ResponseCode PopSlotHandler::ServeRequest(INetRequest *request, IOStream *ostream)
{
    pop_request_t pop_req;
    if (!ParseFromRequest(request, &pop_req))
        return RC_BAD_REQUEST;

    const unsigned int count_limit = std::max(pop_req.count_limit, 1u);
    const unsigned int size_limit = (pop_req.size_limit.get() != nullptr ? *pop_req.size_limit : 0);

    std::unique_ptr<service::ITask*[]> tasks(new service::ITask*[count_limit]);
    if (tasks.get() == nullptr)
        return RC_SERVICE_BUSY;

    unsigned int count = queue_->GetTasks(tasks.get(), count_limit);  // TODO: apply filter pop_req.dest_node
    unsigned int serialized = 0;
    bool succeed = SerializeTasks(count, tasks.get(), size_limit, ostream, &serialized);

    if (!succeed)
        serialized = 0;

    for (unsigned int i=0; i<serialized; ++i)
        delete tasks[i];
    for (unsigned int i=serialized; i<count; ++i)
        queue_->ReAddTask(tasks[i]);
    tasks.reset();

    if (!succeed)
        return RC_SERVICE_BUSY;

    if (serialized > 0)
        queue_->TaskDone(serialized);

    agent_->Info("%d tasks poped from /%s/%s", serialized, agent_->GetServiceName(), slot_name_);
    return RC_OK;
}

bool PopSlotHandler::SerializeTasks(
        unsigned int count, service::ITask* tasks[], unsigned int size_limit,
        IOStream *ostream, unsigned int *serialized)
{
    TlvComposer composer(ostream);
    if (!composer.BeginKey("root"))
        return false;

    // unsigned int serialized_size = 0;
    *serialized = count;
    for (unsigned int i = 0; i < count; ++i)
    {
        service::ITask* task = tasks[i];
        assert(task != nullptr);

        if (!task->AnteSerialize())
            return false;

        TaskMetaImpl *meta = dynamic_cast<TaskMetaImpl*>(task->GetTaskMeta());
        if (!composer.BeginKey("meta"))
            return false;
        if (meta != nullptr)
        {
            if (!meta->Serialize(&composer))
                return false;
        }
        if (composer.EndKey("meta"))
            return false;

        IOStream *os_task = composer.BeginValue("task");
        if (os_task == nullptr)
            return false;
        if (!task->Serialize(os_task))
            return false;
        if (composer.EndValue(os_task) < 0)
            return false;

        if (size_limit != 0 && (unsigned)composer.GetWroteSize() >= size_limit)
        {
            *serialized = i + 1;
            break;
        }
    }

    if (!composer.EndKey("root"))
        return false;
    if (!composer.CheckIntegrity())
        return false;

    return true;
}

}}  // namespace fasmio::container

