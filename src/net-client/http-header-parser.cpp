
#include "./http-header-parser.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace fasmio { namespace net_client {

HttpHeaderParser::HttpHeaderParser(IIStream* http_response) :
    http_response_(http_response),
    buffer_(nullptr),
    buffer_size_(0),
    buffer_begin_(0),
    buffer_end_(0),
    buffer_line_end_(0)
{
}

HttpHeaderParser::~HttpHeaderParser()
{
    if (buffer_ != nullptr)
        delete[] buffer_;
}

bool HttpHeaderParser::GetStatus(int *status_code)
{
    if (!PumpUntilLineEnd())
        return false;

    char* line_begin = buffer_ + buffer_begin_;

    if (0 == memcmp("HTTP/1.", line_begin, 7) &&
        isdigit(line_begin[7]) && ' ' == line_begin[8] &&
        isdigit(line_begin[9]) && isdigit(line_begin[10]) && isdigit(line_begin[11]))
    {
        if (status_code != nullptr)
            *status_code = atoi(line_begin + 9);
        buffer_begin_ = buffer_line_end_;
        return true;
    }

    return false;
}

bool HttpHeaderParser::GetNextHeader(const char **name, const char **value)
{
    if (!PumpUntilLineEnd())
        return false;

    char* line_begin = buffer_ + buffer_begin_;
    long line_length = buffer_line_end_ - buffer_begin_;
    char* line_end = line_begin + line_length;

    if (line_length == 2)
    {
        buffer_begin_ = buffer_line_end_;
        *name = nullptr;
        *value = nullptr;
        return true;
    }

    assert(*(line_end - 2) == '\r');
    assert(*(line_end - 1) == '\n');
    *(line_end - 2) = '\0';

    char* colon = (char*)memchr(line_begin, ':', line_length);
    if (colon == nullptr)
    {
        *name = line_begin;
        *value = line_end - 2;
    }
    else
    {
        *name = line_begin;

        *colon = '\0';
        for (++colon; *colon == ' '; ++colon)
            ;
        *value = colon;
    }

    buffer_begin_ = buffer_line_end_;
    return true;
}

long HttpHeaderParser::Read(void* buff, long size)
{
    if (buffer_begin_ == buffer_end_)
    {
        if (Pump() < 0)
            return -1;
    }

    long bytes = std::min(buffer_end_ - buffer_begin_, size);
    if (bytes > 0)
    {
        memcpy(buff, buffer_ + buffer_begin_, bytes);
        buffer_begin_ += bytes;
    }
    return bytes;
}

bool HttpHeaderParser::PumpUntilLineEnd()
{
    if (buffer_begin_ == buffer_end_)
    {
        if (Pump() <= 0)
            return false;
    }

    while (true)
    {
        char* cfnl = (char*)memmem(buffer_ + buffer_begin_, buffer_end_ - buffer_begin_, "\r\n", 2);
        if (cfnl != nullptr)
        {
            buffer_line_end_ = (cfnl + 2) - buffer_;
            return true;
        }

        if (Pump() <= 0)
            return false;
    }
}

long HttpHeaderParser::Pump()
{
    if (buffer_ == nullptr)
    {
        buffer_size_ = 4096;
        buffer_ = new char[buffer_size_];
    }

    if (buffer_begin_ < buffer_end_)
        memmove(buffer_, buffer_ + buffer_begin_, buffer_end_ - buffer_begin_);
    buffer_begin_ = buffer_end_ = 0;

    long total = 0;
    while (buffer_end_ < buffer_size_)
    {
        long read = http_response_->Read(buffer_ + buffer_end_, buffer_size_ - buffer_end_);
        if (read <= 0)
        {
            if (total == 0 && read < 0)
                return read;
            break;
        }
        total += read;
        buffer_end_ += read;
    }

    return total;
}

}}  // namespace fasmio::net_client

