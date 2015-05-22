
#ifndef COMMON_RANGED_ISTREAM_H_
#define COMMON_RANGED_ISTREAM_H_

#include "interface/stream.h"
#include <istream>
#include <vector>

namespace fasmio { namespace common {

class RangedIStream: public IIStream
{
public:
    explicit RangedIStream(IIStream *is, unsigned int length);
    virtual ~RangedIStream();

private:
    RangedIStream(const RangedIStream &) = delete;
    RangedIStream& operator= (const RangedIStream &) = delete;

public:
    virtual long Read(void* buff, long size);

private:
    IIStream *is_;
    unsigned int length_;
};


class RangedStdIStream: public std::istream
{
public:
    explicit RangedStdIStream(std::istream &is, unsigned int length);
    virtual ~RangedStdIStream();

private:
    RangedStdIStream(const RangedStdIStream &) = delete;
    RangedStdIStream& operator= (const RangedStdIStream &) = delete;

private:
    class streambuf : public std::streambuf
    {
    public:
        explicit streambuf(std::istream &is, unsigned int length);
        virtual ~streambuf();

    protected:
        virtual int_type underflow();

    private:
        std::istream &is_;
        unsigned int length_;
        std::vector<char> buffer_;
    };

private:
    streambuf buff_;
};

}}  // namespace fasmio::common

#endif  // COMMON_RANGED_ISTREAM_H_

