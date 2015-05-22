
#ifndef INTERFACE_RUNTIME_FILE_H_
#define INTERFACE_RUNTIME_FILE_H_

namespace fasmio { namespace runtime_env {

class IFile
{
public:
    virtual ~IFile() {};

public:
    virtual bool Open(const char* path, const char* mode) = 0;
    virtual void Close() = 0;

    virtual long Write(const void* buff, unsigned long size) = 0;
    virtual long Read(void* buff, unsigned long size) = 0;
};

}}  // namespace fasmio::runtime_env

#endif  // INTERFACE_RUNTIME_FILE_H_

