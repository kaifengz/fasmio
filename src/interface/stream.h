
#ifndef INTERFACE_STREAM_H_
#define INTERFACE_STREAM_H_

namespace fasmio {


// IIStream
class IIStream
{
public:
    virtual ~IIStream() {}

public:
    virtual long Read(void* buff, long size) = 0;
};


// IOStream
class IOStream
{
public:
    virtual ~IOStream() {}

public:
    virtual long Write(const void* buff, long size) = 0;
};

}  // namespace fasmio

#endif  // INTERFACE_STREAM_H_

