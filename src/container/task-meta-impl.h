
#ifndef CONTAINER_TASK_META_IMPL_H_
#define CONTAINER_TASK_META_IMPL_H_

#include "service/interface/task-meta.h"
#include "tlv-composer.h"
#include "tlv-parser.h"
#include <istream>
#include <ostream>
#include <vector>

namespace fasmio { namespace container {

class TaskMetaImpl : public service::ITaskMeta
{
public:
    TaskMetaImpl();
    explicit TaskMetaImpl(const char* task_id);
    virtual ~TaskMetaImpl();

public:
    virtual const char*   GetID                ();
    virtual const char*   GetRootID            ();
    virtual const char*   GetParentID          ();
    virtual const char*   GetParentID          (const char*);

public:
    virtual const char*   GetGeneratorNode     ();
    virtual const char*   GetGeneratorService  ();
    virtual double        GetBirthTime         ();
    virtual bool          NeedAck              ();
    virtual unsigned int  GetResendCount       ();
    virtual double        GetResendTime        ();

    virtual bool          SetGeneratorInfo     (const char* node,
                                                const char* service,
                                                double birth_time,
                                                bool need_ack);

    virtual bool          SetResendInfo        (unsigned int resend_count,
                                                double resend_time);

public:
    virtual service::ITaskJourney* GetJourney           ();
    virtual service::IKey*         GetUserData          ();
    virtual bool                   ClearJourney         ();
    virtual bool                   ClearUserData        ();

public:
    virtual service::ITaskMeta*    Clone                ();
    virtual service::ITaskMeta*    Fork                 ();
    virtual service::ITaskMeta*    Fork                 (const char*);

public:
    virtual bool          Serialize            (IOStream*);
    virtual bool          Serialize            (IOStream*, service::TaskMetaFormat);
    virtual bool          UnSerialize          (IIStream*);

public:
    bool    Serialize   (TlvComposer *composer);
    bool    UnSerialize (TlvParser   *parser);

protected:
    void    allocate_id();

private:
    std::vector<std::string> task_ids_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_TASK_META_IMPL_H_

