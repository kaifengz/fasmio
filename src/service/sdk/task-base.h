
#ifndef SERVICE_SDK_TASK_BASE_H_
#define SERVICE_SDK_TASK_BASE_H_

#include "service/interface/key-value.h"
#include "service/interface/service.h"
#include "service/interface/task.h"
#include "service/interface/task-meta.h"
#include <istream>
#include <ostream>

namespace fasmio { namespace service { namespace sdk {

class TaskBase : public ITask
{
public:
    TaskBase();
    virtual ~TaskBase();

public:
    virtual bool        Serialize   (std::ostream &) = 0;
    virtual bool        UnSerialize (std::istream &) = 0;

public:
    virtual ITaskMeta*  GetTaskMeta ();
    virtual bool        SetTaskMeta (ITaskMeta*);

    virtual bool        Serialize   (IOStream *);
    virtual bool        UnSerialize (IIStream *);

    virtual bool        AnteSerialize   ();
    virtual bool        PostUnSerialize ();

private:
    ITaskMeta* meta_;
};

}}}  // namespace fasmio::service::sdk

#endif  // SERVICE_SDK_TASK_BASE_H_

