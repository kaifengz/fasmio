
#include "./stream-impl.h"

namespace fasmio { namespace container {

StreamImpl::StreamImpl() :
    sstream_(),
    length_(0)
{
}

StreamImpl::~StreamImpl()
{
}

long StreamImpl::Read(void* buff, long size)
{
    sstream_.read(reinterpret_cast<char*>(buff), size);
    return sstream_.gcount();
}

long StreamImpl::Write(const void* buff, long size)
{
    sstream_.write(reinterpret_cast<const char*>(buff), size);
    if (sstream_.bad())
        return -1;

    length_ += size;
    return size;
}

unsigned long StreamImpl::GetLength()
{
    return length_;
}

std::iostream& StreamImpl::GetStdStream()
{
    return static_cast<std::iostream&>(sstream_);
}

bool CopyStream(IIStream *from, IOStream *to, unsigned long *copied)
{
    if (from == nullptr || to == nullptr)
        return false;

    unsigned long total = 0;
    bool succeed = true;
    while (true)
    {
        char buff[4096];
        long bytes = from->Read(buff, sizeof(buff));
        if (bytes <= 0)
        {
            succeed = (bytes == 0);
            break;
        }

        long written = 0;
        while (true)
        {
            long count = to->Write(buff+written, bytes-written);
            if (count <= 0)
                break;
            written += count;
        }

        total += written;
        if (written != bytes)
        {
            succeed = false;
            break;
        }
    }

    if (copied != nullptr)
        *copied = total;
    return succeed;
}

}}  // namespace fasmio::container

