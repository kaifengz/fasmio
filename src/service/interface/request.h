
#ifndef SERVICE_INTERFACE_REQUEST_H_
#define SERVICE_INTERFACE_REQUEST_H_

#include "interface/stream.h"

namespace fasmio { namespace service {

class IRequest
{
public:
    virtual ~IRequest() {};

public:
    virtual const char*  GetURL() = 0;
    virtual const char*  GetMethod() = 0;
    virtual const char*  GetParameter(const char*) = 0;
    virtual unsigned int GetParameterCount() = 0;
    virtual const char*  GetParameterName(unsigned int index) = 0;
    virtual const char*  GetParameterValue(unsigned int index) = 0;
};

}}  // namespace fasmio::service

#endif  // SERVICE_INTERFACE_REQUEST_H_

