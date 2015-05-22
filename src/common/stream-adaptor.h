
#ifndef COMMON_STREAM_ADAPTOR_H_
#define COMMON_STREAM_ADAPTOR_H_

#include "interface/stream.h"
#include <istream>
#include <ostream>
#include <vector>

namespace fasmio { namespace common {

class OStreamAdaptor : public std::ostream
{
public:
    explicit OStreamAdaptor(IOStream *os);
    virtual ~OStreamAdaptor();

    void close();

private:
    OStreamAdaptor(const OStreamAdaptor &) = delete;
    OStreamAdaptor& operator= (const OStreamAdaptor &) = delete;

private:
    class streambuf : public std::streambuf
    {
    public:
        explicit streambuf(IOStream *os);
        virtual ~streambuf();

    public:
        void close();

    protected:
        virtual int_type overflow(int_type c);
        virtual int sync();

    private:
        IOStream *os_;
        std::vector<char> buffer_;
    };

private:
    streambuf buff_;
};


class IStreamAdaptor: public std::istream
{
public:
    explicit IStreamAdaptor(IIStream *is);
    virtual ~IStreamAdaptor();

private:
    IStreamAdaptor(const IStreamAdaptor &) = delete;
    IStreamAdaptor& operator= (const IStreamAdaptor &) = delete;

private:
    class streambuf : public std::streambuf
    {
    public:
        explicit streambuf(IIStream* stream);
        virtual ~streambuf();

    protected:
        virtual int_type underflow();

    private:
        IIStream* is_;
        std::vector<char> buffer_;
    };

private:
    streambuf buff_;
};


class StdOStreamAdaptor : public IOStream
{
public:
    explicit StdOStreamAdaptor(std::ostream &os);
    virtual ~StdOStreamAdaptor();

public:
    virtual long Write(const void* buff, long size);

private:
    std::ostream &os_;
};


class StdIStreamAdaptor : public IIStream
{
public:
    explicit StdIStreamAdaptor(std::istream &is);
    virtual ~StdIStreamAdaptor();

public:
    virtual long Read(void* buff, long size);

private:
    std::istream &is_;
};

}}  // namespace fasmio::common

#endif  // COMMON_STREAM_ADAPTOR_H_

