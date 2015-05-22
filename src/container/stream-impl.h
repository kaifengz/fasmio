
#ifndef CONTAINER_STREAM_IMPL_H_
#define CONTAINER_STREAM_IMPL_H_

#include "interface/stream.h"
#include <sstream>

namespace fasmio { namespace container {

class StreamImpl : public IIStream, public IOStream
{
public:
    StreamImpl();
    virtual ~StreamImpl();

public:
    // IIStream
    virtual long Read(void* buff, long size);

    // IOStream
    virtual long Write(const void* buff, long size);

public:
    unsigned long GetLength();
    std::iostream& GetStdStream();

private:
    // TODO: fusion-stream
    std::stringstream sstream_;
    unsigned long length_;
};

bool CopyStream(IIStream *from, IOStream *to, unsigned long *copied = nullptr);

}}  // namespace fasmio::container

#endif  // CONTAINER_STREAM_IMPL_H_

