
#ifndef RUNTIME_ENV_PTHREAD_ENV_FILE_H_
#define RUNTIME_ENV_PTHREAD_ENV_FILE_H_

#include <stdio.h>

#include "interface/runtime-env/file.h"

namespace fasmio { namespace pthread_env {

class File : public runtime_env::IFile
{
public:
    File();
    virtual ~File();

public:
    virtual bool Open(const char* path, const char* mode);
    virtual void Close();

    virtual long Write(const void* buff, unsigned long size);
    virtual long Read(void* buff, unsigned long size);

private:
    FILE* fp_;
};

}}  // namespace fasmio::pthread_env

#endif  // RUNTIME_ENV_PTHREAD_ENV_FILE_H_

