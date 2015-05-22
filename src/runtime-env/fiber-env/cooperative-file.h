
#ifndef RUNTIME_ENV_FIBER_ENV_COOPERATIVE_FILE_H_
#define RUNTIME_ENV_FIBER_ENV_COOPERATIVE_FILE_H_

#include <stdio.h>

#include "interface/runtime-env/file.h"
#include "./cooperative-fd.h"

namespace fasmio { namespace fiber_env {

class CooperativeFile : public runtime_env::IFile
{
public:
    CooperativeFile();
    virtual ~CooperativeFile();

public:
    virtual bool Open(const char* path, const char* mode);
    virtual void Close();

    virtual long Write(const void* buff, unsigned long size);
    virtual long Read(void* buff, unsigned long size);

private:
    FILE* fp_;
    CooperativeFD fd_;
};

}}  // namespace fasmio::fiber_env

#endif  // RUNTIME_ENV_FIBER_ENV_COOPERATIVE_FILE_H_

