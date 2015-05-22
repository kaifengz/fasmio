
#ifndef SERVICE_QUEUE_H_
#define SERVICE_QUEUE_H_

namespace fasmio { namespace service {

class ITask;
class IService;

typedef bool (*TaskFilter)(IService* service, const char* dest_host, const ITask* task);

class IOutQueue
{
public:
    virtual ~IOutQueue() {};

public:
    virtual bool           AddTask    (ITask* task) = 0;

    virtual bool           SetFilter  (TaskFilter filter) = 0;
};


}}  // namespace fasmio::service

#endif  // SERVICE_QUEUE_H_

