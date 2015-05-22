
#include "./ranged-istream.h"
#include <assert.h>

namespace fasmio { namespace common {

RangedIStream::RangedIStream(IIStream *is, unsigned int length) :
    is_(is),
    length_(length)
{
}

RangedIStream::~RangedIStream()
{
}

long RangedIStream::Read(void* buff, long size)
{
    if (length_ <= 0)
        return 0;
    if (is_ == nullptr)
        return -1;

    size = is_->Read(buff, std::min(size, (long)length_));
    if (size > 0)
        length_ -= size;
    return size;
}


RangedStdIStream::RangedStdIStream(std::istream &is, unsigned int length) :
    std::istream(),
    buff_(is, length)
{
    init(&buff_);
}

RangedStdIStream::~RangedStdIStream()
{
}

RangedStdIStream::streambuf::streambuf(std::istream &is, unsigned int length) :
    std::streambuf(),
    is_(is),
    length_(length),
    buffer_(512)
{
    setg(&buffer_[0], &buffer_[0], &buffer_[0]);
    setp(&buffer_[0], &buffer_[0]);
}

RangedStdIStream::streambuf::~streambuf()
{
}

RangedStdIStream::streambuf::int_type RangedStdIStream::streambuf::underflow()
{
    if (gptr() < egptr())
        return traits_type::to_int_type(*gptr());

    if (length_ == 0)
        return traits_type::eof();

    if (!is_.read(&buffer_[0], std::min(buffer_.size(), length_)))
        return traits_type::eof();

    const int bytes = is_.gcount();
    length_ -= bytes;
    setg(&buffer_[0], &buffer_[0], &buffer_[bytes]);
    return traits_type::to_int_type(*gptr());
}

}}  // namespace fasmio::common

