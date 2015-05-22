
#ifndef SERVICE_INTERFACE_SERVICE_H_
#define SERVICE_INTERFACE_SERVICE_H_

#include "interface/logger.h"

namespace fasmio { namespace service {

class IServiceAgent;

class IService
{
public:
    virtual ~IService() {}

public:
    virtual bool Initialize () = 0;
    virtual void Finalize   () = 0;

public:
    virtual void OnConfiguring   () = 0;
    virtual void OnConfigured    () = 0;
    virtual void OnAboutToPause  () = 0;
    virtual void OnPausing       () = 0;
    virtual void OnResuming      () = 0;
    virtual void OnAboutToRetire () = 0;
    virtual void OnRetiring      () = 0;
    virtual void OnRetired       () = 0;

public:
    virtual bool IsOverload  () = 0;
    virtual bool IsUnderload () = 0;
};

}}  // namespace fasmio::service

#endif  // SERVICE_INTERFACE_SERVICE_H_

