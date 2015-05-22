
#include "./stream-adaptor.h"

namespace fasmio { namespace common {

OStreamAdaptor::streambuf::streambuf(IOStream* os) :
    std::streambuf(),
    os_(os),
    buffer_(256)
{
    setp(&buffer_[0], &buffer_[buffer_.size()]);
}

OStreamAdaptor::streambuf::~streambuf()
{
    close();
}

void OStreamAdaptor::streambuf::close()
{
    if (os_ != nullptr)
    {
        sync();
        os_ = nullptr;
    }
}

OStreamAdaptor::streambuf::int_type OStreamAdaptor::streambuf::overflow(int_type c)
{
    if (sync() < 0)
        return traits_type::eof();

    *(pptr()) = c;
    pbump(1);
    return traits_type::not_eof(c);
}

int OStreamAdaptor::streambuf::sync()
{
    if (os_ == nullptr)
        return -1;

    int to_sync = pptr() - pbase();
    if (to_sync <= 0)
        return to_sync;

    long synced = 0;
    while (true)
    {
        long bytes = os_->Write(pbase() + synced, to_sync - synced);
        if (bytes <= 0)
            return -1;

        synced += bytes;
        if (synced == to_sync)
            break;
        else if (synced > to_sync)
            return -1;
    }

    setp(&buffer_[0], &buffer_[buffer_.size()]);
    return to_sync;
}

OStreamAdaptor::OStreamAdaptor(IOStream *os):
    std::ostream(),
    buff_(os)
{
    std::ostream::init(&buff_);
}

OStreamAdaptor::~OStreamAdaptor()
{
}

void OStreamAdaptor::close()
{
    buff_.close();
}



IStreamAdaptor::streambuf::streambuf(IIStream* is) :
    std::streambuf(),
    is_(is),
    buffer_(512)
{
    setg(&buffer_[0], &buffer_[0], &buffer_[0]);
    setp(&buffer_[0], &buffer_[0]);
}

IStreamAdaptor::streambuf::~streambuf()
{
}

IStreamAdaptor::streambuf::int_type IStreamAdaptor::streambuf::underflow()
{
    if (gptr() < egptr())
        return traits_type::to_int_type(*gptr());

    int bytes = is_->Read(&buffer_[0], buffer_.size());
    if (bytes <= 0)
        return traits_type::eof();

    setg(&buffer_[0], &buffer_[0], &buffer_[bytes]);
    return traits_type::to_int_type(*gptr());
}

IStreamAdaptor::IStreamAdaptor(IIStream *is) :
    std::istream(),
    buff_(is)
{
    rdbuf(&buff_);
}

IStreamAdaptor::~IStreamAdaptor()
{
}

StdOStreamAdaptor::StdOStreamAdaptor(std::ostream &os) :
    os_(os)
{
}

StdOStreamAdaptor::~StdOStreamAdaptor()
{
}

long StdOStreamAdaptor::Write(const void* buff, long size)
{
    os_.write(reinterpret_cast<const char*>(buff), size);
    return os_.good() ? size : -1;
}

StdIStreamAdaptor::StdIStreamAdaptor(std::istream &is) :
    is_(is)
{
}

StdIStreamAdaptor::~StdIStreamAdaptor()
{
}

long StdIStreamAdaptor::Read(void* buff, long size)
{
    is_.read(reinterpret_cast<char*>(buff), size);
    return is_.gcount();
}

}}  // namespace fasmio::common

