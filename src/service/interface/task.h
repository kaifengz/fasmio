
#ifndef SERVICE_INTERFACE_TASK_H_
#define SERVICE_INTERFACE_TASK_H_

#include "interface/stream.h"

namespace fasmio { namespace service {

class ITaskMeta;

class ITask
{
public:
    virtual ~ITask() {};

public:
    virtual ITaskMeta*  GetTaskMeta ()           = 0;
    virtual bool        SetTaskMeta (ITaskMeta*) = 0;

    virtual bool        Serialize   (IOStream*) = 0;
    virtual bool        UnSerialize (IIStream*) = 0;

    virtual bool        AnteSerialize   () = 0;
    virtual bool        PostUnSerialize () = 0;
};

}}  // namespace fasmio::service

#endif  // SERVICE_INTERFACE_TASK_H_

