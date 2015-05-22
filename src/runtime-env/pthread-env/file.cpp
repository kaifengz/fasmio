
#include "./file.h"

namespace fasmio { namespace pthread_env {

File::File() :
    fp_(nullptr)
{
}

File::~File()
{
    Close();
}

bool File::Open(const char* path, const char* mode)
{
    if (fp_ != nullptr)
        return false;

    fp_ = fopen(path, mode);
    return (fp_ != nullptr);
}

void File::Close()
{
    if (fp_ != nullptr)
    {
        fclose(fp_);
        fp_ = nullptr;
    }
}

long File::Write(const void* buff, unsigned long size)
{
    if (fp_ == nullptr)
        return -1;

    return fwrite(buff, 1, size, fp_);
}

long File::Read(void* buff, unsigned long size)
{
    if (fp_ == nullptr)
        return -1;

    return fread(buff, 1, size, fp_);
}

}}  // namespace fasmio::pthread_env

