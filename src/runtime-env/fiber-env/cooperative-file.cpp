
#include "./cooperative-file.h"

#include <assert.h>

namespace fasmio { namespace fiber_env {

CooperativeFile::CooperativeFile() :
    fp_(nullptr),
    fd_()
{
}

CooperativeFile::~CooperativeFile()
{
    Close();
}

bool CooperativeFile::Open(const char* path, const char* mode)
{
    if (fp_ != nullptr)
        return false;

    fp_ = fopen(path, mode);
    if (fp_ == nullptr)
        return false;

    if (0 != fd_.Attach(fileno(fp_)))
    {
        fclose(fp_);
        fp_ = nullptr;
        return false;
    }

    return true;
}

void CooperativeFile::Close()
{
    if (fp_ != nullptr)
    {
        fd_.Dettach();

        fclose(fp_);
        fp_ = nullptr;
    }
}

long CooperativeFile::Write(const void* buff, unsigned long size)
{
    if (fp_ == nullptr)
        return -1;

    return fd_.Write(buff, size, "file.write");
}

long CooperativeFile::Read(void* buff, unsigned long size)
{
    if (fp_ == nullptr)
        return -1;

    return fd_.Read(buff, size, "file.read");
}

}}  // namespace fasmio::fiber_env
