
#ifndef SERVICE_INTERFACE_TASK_META_H_
#define SERVICE_INTERFACE_TASK_META_H_

#include "interface/stream.h"

namespace fasmio { namespace service {

enum TaskMetaFormat
{
    TMF_BINARY = 0,
    TMF_XML,
    // TODO
//  TMF_JSON,
//  TMF_THRIFT,
};

class IKey;
class ITaskJourney;

class ITaskMeta
{
public:
    virtual ~ITaskMeta() {};

public:
    virtual const char*   GetID                ()            = 0;
    virtual const char*   GetRootID            ()            = 0;
    virtual const char*   GetParentID          ()            = 0;
    virtual const char*   GetParentID          (const char*) = 0;

public:
    virtual const char*   GetGeneratorNode     ()            = 0;
    virtual const char*   GetGeneratorService  ()            = 0;
    virtual double        GetBirthTime         ()            = 0;
    virtual bool          NeedAck              ()            = 0;
    virtual unsigned int  GetResendCount       ()            = 0;
    virtual double        GetResendTime        ()            = 0;

    virtual bool          SetGeneratorInfo     (const char* node,
                                                const char* service,
                                                double birth_time,
                                                bool need_ack) = 0;

    virtual bool          SetResendInfo        (unsigned int resend_count,
                                                double resend_time) = 0;

public:
    virtual ITaskJourney* GetJourney           ()            = 0;
    virtual IKey*         GetUserData          ()            = 0;
    virtual bool          ClearJourney         ()            = 0;
    virtual bool          ClearUserData        ()            = 0;

public:
    virtual ITaskMeta*    Clone                ()            = 0;
    virtual ITaskMeta*    Fork                 ()            = 0;
    virtual ITaskMeta*    Fork                 (const char*) = 0;

public:
    virtual bool          Serialize            (IOStream*)   = 0;
    virtual bool          Serialize            (IOStream*, TaskMetaFormat) = 0;
    virtual bool          UnSerialize          (IIStream*)   = 0;
};

}}  // namespace fasmio::service

#endif  // SERVICE_INTERFACE_TASK_META_H_

