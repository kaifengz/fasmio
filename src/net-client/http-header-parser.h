
#ifndef NET_CLIENT_HTTP_HEADER_PARSER_H_
#define NET_CLIENT_HTTP_HEADER_PARSER_H_

#include "interface/stream.h"
#include "interface/runtime-env.h"

namespace fasmio { namespace net_client {

class HttpHeaderParser : public IIStream
{
public:
    explicit HttpHeaderParser(IIStream* http_response);
    ~HttpHeaderParser();

public:
    bool GetStatus(int *status_code);
    bool GetNextHeader(const char **name, const char **value);

public:
    virtual long Read(void* buff, long size);

private:
    bool PumpUntilLineEnd();
    long Pump();

private:
    IIStream *http_response_;    
    char *buffer_;
    long buffer_size_;
    long buffer_begin_;
    long buffer_end_;
    long buffer_line_end_;
};

}}  // namespace fasmio::net_client

#endif  // NET_CLIENT_HTTP_HEADER_PARSER_H_

