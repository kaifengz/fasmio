
#ifndef SERVICE_SDK_GENERAL_XSD_TASK_H_
#define SERVICE_SDK_GENERAL_XSD_TASK_H_

#include "./task-base.h"

namespace fasmio { namespace service { namespace sdk {

template <typename XsdDataType>
class GeneralXsdTask : public TaskBase
{
public:
    typedef XsdDataType xsd_data_t;

public:
    GeneralXsdTask() :
        data_()
    {
    }

    explicit GeneralXsdTask(const XsdDataType &data) :
        data_(data)
    {
    }

public:
    XsdDataType* operator-> ()
    {
        return &data_;
    }

    XsdDataType& operator* ()
    {
        return data_;
    }

    XsdDataType* get()
    {
        return &data_;
    }

public:
    virtual bool Serialize(std::ostream &os)
    {
        return XsdSerialize(os, data_);
    }

    virtual bool UnSerialize(std::istream &is)
    {
        return XsdUnserialize(is, data_);
    }

public:
    static ITask* Allocator(IService*)
    {
        return new GeneralXsdTask<XsdDataType>();
    }

private:
    XsdDataType data_;
};

}}}  // namespace fasmio::service::sdk

#endif  // SERVICE_SDK_GENERAL_XSD_TASK_H_

