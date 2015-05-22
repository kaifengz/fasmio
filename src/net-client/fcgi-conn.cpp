
#include "./fcgi-conn.h"
#include "./http-header-parser.h"
#include <assert.h>
#include <string.h>
#include <memory>
#include <sstream>
#include <string>

#define MERGE2(name) (((name ## B1) << 8) | (name ## B0))
#define SPLIT2(name, value) \
    do { \
        unsigned int v = (value); \
        assert(v <= 0xFFFFu); \
        name ## B1 = (v >> 8); \
        name ## B0 = (v & 0xFF); \
    } while (0)
#define SPLIT4(name, value) \
    do { \
        unsigned int v = (value); \
        name ## B3 = (v >> 24); \
        name ## B2 = ((v >> 16) & 0xFF); \
        name ## B1 = ((v >> 8) & 0xFF); \
        name ## B0 = (v & 0xFF); \
    } while (0)


namespace fasmio { namespace net_client {

const unsigned short kDefaultFcgiPort = 9000;
const unsigned long kContentLengthNotSpecified = 0xFFFFFFFF;

FcgiConn::FcgiConn(IRuntimeEnv* env, ILogger* logger) :
    ConnBase(env, logger, "FcgiClient"),
    state_(FS_INIT),
    fcgi_rw_buff_(),
    fcgi_stdin_write_end_(0),
    fcgi_stdout_read_begin_(0),
    fcgi_stdout_read_end_(0),
    fcgi_stdout_to_read_(0),
    fcgi_stdout_padding_(0)
{
}

FcgiConn::~FcgiConn()
{
    Close(nullptr);
}

IOStream* FcgiConn::StartRequest(const char* method, const char* url, unsigned long content_length)
{
    if (sock_ == nullptr || method == nullptr || url == nullptr)
        return nullptr;

    if (!SendBeginRequest())
        return nullptr;
    if (!SendParams(method, url, content_length))
        return nullptr;

    return static_cast<IOStream*>(this);
}

bool FcgiConn::SendBeginRequest()
{
    if (sock_ == nullptr || state_ != FS_INIT)
        return nullptr;

    FCGI_BeginRequestRecord record;

    memset(&record, 0, sizeof(record));
    record.header.version = FCGI_VERSION_1;
    record.header.type = FCGI_BEGIN_REQUEST;
    SPLIT2(record.header.requestId, 1);
    SPLIT2(record.header.contentLength, sizeof(record.body));
    SPLIT2(record.body.role, FCGI_RESPONDER);

    if (sizeof(record) != sock_->SendAll(&record, sizeof(record)))
    {
        PLOG("Failed to send the FCGI_BEGIN_REQUEST");
        OnError();
        return false;
    }

    PLOG("Sent: FCGI_BEGIN_REQUEST");
    state_ = FS_BEGIN_REQUEST_SENT;
    return true;
}

bool FcgiConn::SendParams(const char* method, const char* url, unsigned long content_length)
{
    if (sock_ == nullptr || state_ != FS_BEGIN_REQUEST_SENT)
        return false;

    std::ostringstream oss;

#define PUT_NAME_VALUE_PAIR(name, value)    \
    do { \
        long name_len = strlen(name); \
        long value_len = strlen(value); \
        if (name_len >= 0x80 || value_len >= 0x80) \
            return false; \
        oss.put((char)name_len); \
        oss.put((char)value_len); \
        oss.write(name, name_len); \
        oss.write(value, value_len); \
    } while (0)

    PUT_NAME_VALUE_PAIR("REQUEST_METHOD", method);
    PUT_NAME_VALUE_PAIR("REQUEST_URI", url);
    if (content_length != kContentLengthNotSpecified)
    {
        char buff[32];
        snprintf(buff, sizeof(buff), "%lu", content_length);
        PUT_NAME_VALUE_PAIR("CONTENT_LENGTH", buff);
    }
#undef PUT_NAME_VALUE_PAIR

    std::string body = oss.str();

    FCGI_Header header;
    memset(&header, 0, sizeof(header));
    header.version = FCGI_VERSION_1;
    header.type = FCGI_PARAMS;
    SPLIT2(header.requestId, 1);
    SPLIT2(header.contentLength, body.size());

    if (sizeof(header) != sock_->SendAll(&header, sizeof(header)) ||
        (long)body.size() != sock_->SendAll(body.c_str(), body.size()))
    {
        PLOG("Failed to send the FCGI_PARAMS with length = %d", body.size());
        OnError();
        return false;
    }
    PLOG("Sent: FCGI_PARAMS (length = %d)", body.size());

    SPLIT2(header.contentLength, 0);
    if (sizeof(header) != sock_->SendAll(&header, sizeof(header)))
    {
        PLOG("Failed to send the FCGI_PARAMS with length = 0");
        OnError();
        return false;
    }

    PLOG("Sent: FCGI_PARAMS (length = 0)");
    state_ = FS_PARAMS_SENT;
    return true;
}

bool FcgiConn::EndRequest(IOStream **is)
{
    if (sock_ == nullptr)
        return false;
    if (state_ != FS_PARAMS_SENT && state_ != FS_STDIN_SENT)
        return false;

    if (!FlushStdin(nullptr, 0, true))
        return false;

    state_ = FS_STDIN_END;
    return true;
}

bool FcgiConn::Close(IIStream **is)
{
    if (sock_ != nullptr)
        ReceiveEndRequest();

    return ConnBase::Close(is);
}

long FcgiConn::Write(const void* buff, long size)
{
    if (sock_ == nullptr)
        return -1;
    if (state_ != FS_PARAMS_SENT && state_ != FS_STDIN_SENT)
        return false;

    if (fcgi_stdin_write_end_ + size < sizeof(fcgi_rw_buff_))
    {
        memcpy(fcgi_rw_buff_ + fcgi_stdin_write_end_, buff, size);
        fcgi_stdin_write_end_ += size;
    }
    else
    {
        if (!FlushStdin(buff, size, false))
            return -1;
        assert(fcgi_stdin_write_end_ == 0);
    }

    state_ = FS_STDIN_SENT;
    return size;
}

bool FcgiConn::FlushStdin(const void* buff, long size, bool terminate_stdin)
{
    FCGI_Header header;
    memset(&header, 0, sizeof(header));
    header.version = FCGI_VERSION_1;
    header.type = FCGI_STDIN;
    SPLIT2(header.requestId, 1);

    if (fcgi_stdin_write_end_ + size > 0)
    {
        SPLIT2(header.contentLength, fcgi_stdin_write_end_ + size);

        if (sizeof(header) != sock_->SendAll(&header, sizeof(header)) ||
            (fcgi_stdin_write_end_ > 0 && (long)fcgi_stdin_write_end_ != sock_->SendAll(fcgi_rw_buff_, fcgi_stdin_write_end_)) ||
            (buff != nullptr && size > 0 && size != sock_->SendAll(buff, size)))
        {
            PLOG("Failed to send the FCGI_STDIN with length = %d", fcgi_stdin_write_end_ + size);
            OnError();
            return false;
        }

        PLOG("Sent: FCGI_STDIN (length = %d)", fcgi_stdin_write_end_ + size);
        fcgi_stdin_write_end_ = 0;
    }

    if (terminate_stdin)
    {
        SPLIT2(header.contentLength, 0);
        if (sizeof(header) != sock_->SendAll(&header, sizeof(header)))
        {
            PLOG("Failed to send the FCGI_STDIN with length = 0");
            OnError();
            return false;
        }

        PLOG("Sent: FCGI_STDIN (length = 0)");
    }

    return true;
}

long FcgiConn::Read(void* buff, long size)
{
    if (sock_ == nullptr)
        return -1;
    if (state_ == FS_STDOUT_END)
        return 0;
    if (state_ != FS_STDIN_END && state_ != FS_STDOUT_RECEIVED)
        return -1;

    int copied = 0;

    while (true)
    {
        assert(fcgi_stdout_read_end_ >= fcgi_stdout_read_begin_);
        if (fcgi_stdout_read_end_ > fcgi_stdout_read_begin_)
        {
            long bytes = fcgi_stdout_read_end_ - fcgi_stdout_read_begin_;
            if (size <= bytes)
            {
                memcpy(buff, fcgi_rw_buff_ + fcgi_stdout_read_begin_, size);
                fcgi_stdout_read_begin_ += size;
                return copied + size;
            }

            memcpy(buff, fcgi_rw_buff_ + fcgi_stdout_read_begin_, bytes);
            buff = (char*)buff + bytes;
            size -= bytes;
            copied += bytes;
        }
        fcgi_stdout_read_begin_ = 0;
        fcgi_stdout_read_end_   = 0;

        if (fcgi_stdout_to_read_ > 0)
        {
            long to_read = std::min((unsigned int)fcgi_stdout_to_read_, sizeof(fcgi_rw_buff_));
            long read = sock_->ReceiveAll(fcgi_rw_buff_, to_read);
            if (read <= 0)
            {
                mlogger_.Error("Unexpected end of stream");
                goto error;
            }
            fcgi_stdout_to_read_ -= read;
            fcgi_stdout_read_end_ += read;
            continue;
        }
        assert(fcgi_stdout_to_read_ == 0);

        if (fcgi_stdout_padding_ > 0)
        {
            char padding[256];
            assert(fcgi_stdout_padding_ <= sizeof(padding));
            long read = sock_->ReceiveAll(padding, fcgi_stdout_padding_);
            if (read != (long)fcgi_stdout_padding_)
            {
                mlogger_.Error("Unexpected end of stream");
                goto error;
            }
        }

        PLOG("Waiting for FCGI header of FCGI_STDOUT");
        FCGI_Header header;
        if (sizeof(header) != sock_->ReceiveAll(&header, sizeof(header)))
        {
            mlogger_.Error("Unexpected end of stream");
            goto error;
        }

        if (header.version != FCGI_VERSION_1)
        {
            mlogger_.Error("Unknown FCGI version %d", header.version);
            goto error;
        }

        if (header.type != FCGI_STDOUT)
        {
            mlogger_.Error("Unexpected FCGI type %d, expected FCGI_STDOUT", header.type);
            goto error;
        }

        if (MERGE2(header.requestId) != 1)
        {
            mlogger_.Error("Unexpected requestId %d, expected 1", MERGE2(header.requestId));
            goto error;
        }

        fcgi_stdout_to_read_ = MERGE2(header.contentLength);
        fcgi_stdout_padding_ = header.paddingLength;
        PLOG("Received: FCGI_STDOUT with length = %d", fcgi_stdout_to_read_);
        if (fcgi_stdout_to_read_ == 0)
        {
            state_ = FS_STDOUT_END;
            return copied;
        }

        state_ = FS_STDOUT_RECEIVED;
    }

error:
    OnError();
    return -1;
}

bool FcgiConn::ReceiveEndRequest()
{
    if (sock_ == nullptr)
        return false;
    if (state_ != FS_STDIN_END)
        return false;

    FCGI_EndRequestRecord record;
    if (sizeof(record) != sock_->ReceiveAll(&record, sizeof(record)))
    {
        mlogger_.Error("Unexpected end of stream, expected FCGI_END_REQUEST");
        goto error;
    }

    if (record.header.version != FCGI_VERSION_1)
    {
        mlogger_.Error("Unknown FCGI version %d", record.header.version);
        goto error;
    }

    if (record.header.type != FCGI_END_REQUEST)
    {
        mlogger_.Error("Unexpected FCGI type %d, expected FCGI_STDOUT", record.header.type);
        goto error;
    }

    if (MERGE2(record.header.requestId) != 1)
    {
        mlogger_.Error("Unexpected requestId %d, expected 1", MERGE2(record.header.requestId));
        goto error;
    }

    state_ = FS_END_REQUEST_RECEIVED;
    return true;

error:
    OnError();
    return false;
}

unsigned short FcgiConn::GetDefaultPort()
{
    return kDefaultFcgiPort;
}

void FcgiConn::OnError()
{
    state_ = FS_ERROR;
    ConnBase::OnError();
}

}}  // namespace fasmio::net_client

