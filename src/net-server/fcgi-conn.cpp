
#include "./fcgi-conn.h"
#include "./fcgi-server.h"
#include <assert.h>
#include <string.h>

#if 0
#   define PLOG         mlogger_.Verbose
#else
#   define PLOG         plog
    inline void plog(const char* format, ...) {}
#endif

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

namespace fasmio { namespace net_server {

FcgiConnHandler::FcgiConnHandler(FcgiServer* server, common::ModuleLogger *mlogger, runtime_env::ITCPSocket *sock, const char* addr) :
    server_(server),
    mlogger_(*mlogger),
    sock_(sock),
    addr_(addr),
    status_(CS_CONN_INIT),
    curr_req_id_(0),
    fcgi_flags_(0),
    fcgi_sparams_(),
    fcgi_dparams_(0),
    fcgi_dparams_size_(0),
    fcgi_params_(fcgi_sparams_),
    params_(),
    request_url_(),
    request_method_(),
    fcgi_rw_buff_(),
    fcgi_stdin_read_begin_(0),
    fcgi_stdin_read_end_(0),
    fcgi_stdin_to_read_(0),
    fcgi_stdin_padding_(0),
    fcgi_stdout_write_end_(0)
{
}

FcgiConnHandler::~FcgiConnHandler()
{
    delete sock_;

    if (fcgi_dparams_ != 0)
        delete[] fcgi_dparams_;
}

void FcgiConnHandler::HandleConnection()
{
    PLOG("Connection start");
    while (true)
    {
        PLOG("Waiting for FCGI header");
        FCGI_Header header;
        if (sizeof(header) != sock_->ReceiveAll(&header, sizeof(header)))
        {
            mlogger_.Error("Unexpected end of stream");
            break;
        }

        if (header.version != FCGI_VERSION_1)
        {
            mlogger_.Error("Unknown FCGI version %d", header.version);
            break;
        }

        bool rc = false;

        PLOG("Process %s (%d)", FcgiTypeToName(header.type), header.type);
        switch (header.type)
        {
        case FCGI_BEGIN_REQUEST:
            if (status_ == CS_CONN_INIT || status_ == CS_END_REQUEST)
                rc = DoBeginRequest(header);
            else
                mlogger_.Error("Unexpected FCGI_BEGIN_REQUEST");
            break;

        case FCGI_PARAMS:
            if (status_ == CS_BEGIN_REQUEST || status_ == CS_PARAMS)
                rc = DoParams(header);
            else
                mlogger_.Error("Unexpected FCGI_PARAMS");
            break;

        default:
            mlogger_.Error("Unknown FCGI type %s (%d)", FcgiTypeToName(header.type), header.type);
            break;
        }

        if (rc && header.paddingLength > 0)
        {
            PLOG("Eat padding %d bytes", header.paddingLength);
            rc = EatPadding(header.paddingLength);
        }

        if (rc && status_ == CS_PARAMS_END)
        {
            // params done, pass the request to container

            PLOG("Pass request to container");
            if (!(rc = PrepareRequest()))
                break;

            ResponseCode resp_code;
            INetRequest *request = static_cast<INetRequest*>(this);
            IOStream *ostream = static_cast<IOStream*>(this);

            if (RC_OK == (resp_code = server_->PrepareRequest(request)))
                resp_code = server_->ServeRequest(request, ostream);

            // send the final two STDOUT and END_REQUEST
            if (!(rc = EndStdout()))
                break;
            if (!(rc = EndRequest(resp_code)))
                break;
            status_ = CS_END_REQUEST;
            PLOG("Request done");

            if ((fcgi_flags_ & FCGI_KEEP_CONN) == 0)
            {
                PLOG("End connection as FCGI_KEEP_CONN not set");
                break;
            }
        }

        if (!rc)
            break;
    }

    PLOG("Connection terminate");
}

const char* FcgiConnHandler::GetURL()
{
    return request_url_.c_str();
}

const char* FcgiConnHandler::GetMethod()
{
    return request_method_.c_str();
}

IIStream* FcgiConnHandler::GetContent()
{
    return static_cast<IIStream*>(this);
}

long FcgiConnHandler::Read(void* buff, long size)
{
    if (status_ == CS_STDIN_END)
        return 0;
    if (status_ != CS_PARAMS_END && status_ != CS_STDIN)
        return -1;

    int copied = 0;

    while (true)
    {
        assert(fcgi_stdin_read_end_ >= fcgi_stdin_read_begin_);
        if (fcgi_stdin_read_end_ > fcgi_stdin_read_begin_)
        {
            long bytes = fcgi_stdin_read_end_ - fcgi_stdin_read_begin_;
            if (size <= bytes)
            {
                memcpy(buff, fcgi_rw_buff_ + fcgi_stdin_read_begin_, size);
                fcgi_stdin_read_begin_ += size;
                return copied + size;
            }

            memcpy(buff, fcgi_rw_buff_ + fcgi_stdin_read_begin_, bytes);
            buff = (char*)buff + bytes;
            size -= bytes;
            copied += bytes;
        }
        fcgi_stdin_read_begin_ = 0;
        fcgi_stdin_read_end_   = 0;

        if (fcgi_stdin_to_read_ > 0)
        {
            long to_read = std::min((unsigned int)fcgi_stdin_to_read_, sizeof(fcgi_rw_buff_));
            long read = sock_->ReceiveAll(fcgi_rw_buff_, to_read);
            if (read <= 0)
            {
                mlogger_.Error("Unexpected end of stream");
                goto error;
            }
            fcgi_stdin_to_read_ -= read;
            fcgi_stdin_read_end_ += read;
            continue;
        }
        assert(fcgi_stdin_to_read_ == 0);

        if (fcgi_stdin_padding_ > 0)
        {
            char padding[256];
            assert(fcgi_stdin_padding_ <= sizeof(padding));
            long read = sock_->ReceiveAll(padding, fcgi_stdin_padding_);
            if (read != (long)fcgi_stdin_padding_)
            {
                mlogger_.Error("Unexpected end of stream");
                goto error;
            }
        }

        PLOG("Waiting for FCGI header of FCGI_STDIN");
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

        if (header.type != FCGI_STDIN)
        {
            mlogger_.Error("Unexpected FCGI type %d, expect FCGI_STDIN", header.type);
            goto error;
        }

        fcgi_stdin_to_read_ = MERGE2(header.contentLength);
        fcgi_stdin_padding_ = header.paddingLength;
        PLOG("FCGI_STDIN with contentLength = %d", fcgi_stdin_to_read_);
        if (fcgi_stdin_to_read_ == 0)
        {
            status_ = CS_STDIN_END;
            return copied;
        }
    }

error:
    status_ = CS_ERROR;
    if (copied > 0)
        return copied;
    else
        return -1;
}

long FcgiConnHandler::Write(const void* buff, long size)
{
    if (status_ != CS_STDOUT && !StartStdout())
        return false;

    FCGI_Header &header = *reinterpret_cast<FCGI_Header*>(fcgi_rw_buff_);

    assert(sizeof(fcgi_rw_buff_) <= 0xFFFFul);
    assert(status_ == CS_STDOUT && fcgi_stdout_write_end_ >= sizeof(header));

    long remain = size;
    while (remain > 0)
    {
        unsigned long seg_data_len = std::min(fcgi_stdout_write_end_ - sizeof(header) + remain, 0xFFFFul);
        unsigned long seg_total_len = seg_data_len + sizeof(header);
        if (seg_total_len < sizeof(fcgi_rw_buff_))
        {
            memcpy(fcgi_rw_buff_ + fcgi_stdout_write_end_, buff, seg_total_len - fcgi_stdout_write_end_);
            fcgi_stdout_write_end_ = seg_total_len;
            break;
        }
        else
        {
            PLOG("Sending FCGI_STDOUT with contentLength = %d", seg_data_len);
            SPLIT2(header.contentLength, seg_data_len);

            long len_in_buff = seg_total_len - fcgi_stdout_write_end_;
            if ((long)fcgi_stdout_write_end_ != sock_->SendAll(fcgi_rw_buff_, fcgi_stdout_write_end_) ||
                len_in_buff != sock_->SendAll(buff, len_in_buff))
            {
                mlogger_.Error("Socket closed unexpectly");
                return -1;
            }
            buff = ((unsigned char*)buff) + len_in_buff;
            remain -= len_in_buff;
            fcgi_stdout_write_end_ = sizeof(header);
        }
    }

    return size;
}

bool FcgiConnHandler::DoBeginRequest(const FCGI_Header &header)
{
    unsigned int request_id = MERGE2(header.requestId);
    unsigned int content_len = MERGE2(header.contentLength);

    FCGI_BeginRequestBody body;
    if (content_len != sizeof(body))
    {
        mlogger_.Error("Unexpected content-length of FCGI_BEGIN_REQUEST, expected %d, got %d",
                    sizeof(body), content_len);
        return false;
    }
    if (sizeof(body) != sock_->ReceiveAll(&body, sizeof(body)))
    {
        mlogger_.Error("Unexpected end of stream");
        return false;
    }

    int role = MERGE2(body.role);
    if (role != FCGI_RESPONDER)
    {
        mlogger_.Error("Unexpected role of FCGI_BeginRequestBody, expected FCGI_RESPONDER, got %d",
                    role);
        return false;
    }

    curr_req_id_ = request_id;
    fcgi_flags_  = body.flags;
    status_      = CS_BEGIN_REQUEST;
    PLOG("FCGI_BEGIN_REQUEST with requestId = %d, flags = 0x%X", curr_req_id_, fcgi_flags_);
    return true;
}

bool FcgiConnHandler::DoParams(const FCGI_Header &header)
{
    unsigned int request_id = MERGE2(header.requestId);
    unsigned int content_len = MERGE2(header.contentLength);

    if (request_id != curr_req_id_)
    {
        mlogger_.Error("Unexpected change of request-id, expected %d, got %d",
                    curr_req_id_, request_id);
        return false;
    }

    if (content_len == 0)
    {
        status_ = CS_PARAMS_END;
        return true;
    }

    if (content_len <= sizeof(fcgi_sparams_))
        fcgi_params_ = fcgi_sparams_;
    else if (content_len <= fcgi_dparams_size_)
    {
        assert(fcgi_dparams_ != 0);
        fcgi_params_ = fcgi_dparams_;
    }
    else
    {
        if (fcgi_dparams_ != 0)
            delete[] fcgi_dparams_;
        fcgi_dparams_ = new unsigned char[content_len];
        fcgi_dparams_size_ = content_len;
        fcgi_params_ = fcgi_dparams_;
    }

    if ((int)content_len != sock_->ReceiveAll(fcgi_params_, content_len))
    {
        mlogger_.Error("Unexpected end of stream");
        return false;
    }

    if (!ParseParams(fcgi_params_, content_len))
    {
        mlogger_.Error("FCGI_PARAMS body format error");
        return false;
    }

    status_ = CS_PARAMS;
    return true;
}

bool FcgiConnHandler::PrepareRequest()
{
    if (!PrepareRequestParam("REQUEST_URI", request_url_))
        return false;

    if (!PrepareRequestParam("REQUEST_METHOD", request_method_))
        return false;

    fcgi_stdin_read_begin_ = 0;
    fcgi_stdin_read_end_   = 0;
    fcgi_stdin_to_read_    = 0;
    return true;
}

bool FcgiConnHandler::PrepareRequestParam(const char* name, std::string &value)
{
    for (Params::const_iterator iter = params_.begin();
        iter != params_.end(); ++iter)
    {
        if (0 == strncmp(name, reinterpret_cast<const char*>(iter->name_.pos_), iter->name_.length_))
        {
            std::string tmp(reinterpret_cast<const char*>(iter->value_.pos_), iter->value_.length_);
            value.swap(tmp);
            return true;
        }
    }

    return false;
}

bool FcgiConnHandler::EatPadding(unsigned int padding_len)
{
    while (padding_len > 0)
    {
        char padding[4096];
        int len = std::min(sizeof(padding), padding_len);
        if ((int)len != sock_->ReceiveAll(padding, len))
        {
            mlogger_.Error("Unexpected end of stream");
            return false;
        }

        padding_len -= len;
    }

    return true;
}

bool FcgiConnHandler::EatStdin()
{
    long total = 0;
    while (true)
    {
        char buff[4096];
        long read = Read(buff, sizeof(buff));
        if (read == 0)
        {
            if (total > 0)
                mlogger_.Verbose("%d bytes still in buffer", total);
            return (status_ == CS_STDIN_END);
        }
        else if (read < 0)
            return false;
        total += read;
    }
}

bool FcgiConnHandler::StartStdout()
{
    if (status_ == CS_PARAMS_END || status_ == CS_STDIN)
    {
        PLOG("Eat hanging stdin");
        if (!EatStdin())
            return false;
        assert(status_ == CS_STDIN_END);
    }

    if (status_ == CS_STDIN_END)
    {
        PLOG("Start stdout");

        FCGI_Header &header = *reinterpret_cast<FCGI_Header*>(fcgi_rw_buff_);
        header.version = FCGI_VERSION_1;
        header.type = FCGI_STDOUT;
        SPLIT2(header.requestId, curr_req_id_);
        header.paddingLength = 0;
        header.reserved = 0;

        static const char* http_headers = "Content-type: text/html\r\n\r\n";
        const unsigned long http_headers_len = 27;

        memcpy(fcgi_rw_buff_ + sizeof(header), http_headers, http_headers_len);
        fcgi_stdout_write_end_ = sizeof(header) + http_headers_len;
        status_ = CS_STDOUT;
    }

    return status_ == CS_STDOUT;
}

bool FcgiConnHandler::EndStdout()
{
    if (status_ != CS_STDOUT && !StartStdout())
        return false;

    FCGI_Header &header = *reinterpret_cast<FCGI_Header*>(fcgi_rw_buff_);
    assert(status_ == CS_STDOUT && fcgi_stdout_write_end_ >= sizeof(header));

    while (true)
    {
        unsigned long contentLength = fcgi_stdout_write_end_ - sizeof(header);
        PLOG("Sending FCGI_STDOUT with contentLength = %d", contentLength);
        SPLIT2(header.contentLength, contentLength);
        if ((long)fcgi_stdout_write_end_ != sock_->SendAll(fcgi_rw_buff_, fcgi_stdout_write_end_))
        {
            mlogger_.Error("Socket closed unexpectly");
            return false;
        }

        if (contentLength == 0)
            break;
        fcgi_stdout_write_end_ = sizeof(header);
    }

    return true;
}

bool FcgiConnHandler::EndRequest(ResponseCode resp_code)
{
    PLOG("Sending FCGI_END_REQUEST which requestId = %d, response code = %d",
        curr_req_id_, resp_code);

    FCGI_EndRequestRecord record;
    FCGI_Header &header = record.header;
    FCGI_EndRequestBody &body = record.body;
    memset(&record, 0, sizeof(record));

    header.version = FCGI_VERSION_1;
    header.type = FCGI_END_REQUEST;
    SPLIT2(header.requestId, curr_req_id_);
    SPLIT2(header.contentLength, sizeof(body));
    header.paddingLength = 0;

    SPLIT4(body.appStatus, resp_code);
    body.protocolStatus = FCGI_REQUEST_COMPLETE;

    if (sizeof(record) != sock_->SendAll(&record, sizeof(record)))
    {
        mlogger_.Error("Socket closed unexpectly");
        return false;
    }

    return true;
}

bool FcgiConnHandler::ParseParams(const unsigned char* params, unsigned int params_len)
{
    params_.clear();
    params_.reserve(8);

#define GET_PARAM_FIELD_LENGTH(length)      \
    do {                                    \
        if (params[0] <= 127) {             \
            length = params[0];             \
            ++params;                       \
            --params_len;                   \
        } else {                            \
            if (params_len < 4)             \
                return false;               \
            name_len = (((params[0] & 0x7F) << 24) | (params[1] << 16) | (params[2] << 8) | params[3]); \
            params += 4;                    \
            params_len -= 4;                \
        }                                   \
    } while (0)

    while (params_len > 0)
    {
        unsigned int name_len, value_len;
        GET_PARAM_FIELD_LENGTH(name_len);
        GET_PARAM_FIELD_LENGTH(value_len);
        if (params_len < name_len + value_len)
            return false;

        params_.resize( params_.size() + 1 );
        Param &param = params_.back();
        param.name_.pos_ = params;
        param.name_.length_ = name_len;
        param.value_.pos_ = params + name_len;
        param.value_.length_ = value_len;

        params += name_len + value_len;
        params_len -= name_len + value_len;

        PLOG("params[\"%.*s\"] = \"%.*s\"",
                param.name_.length_, param.name_.pos_,
                param.value_.length_, param.value_.pos_);
    }

    return true;
}

const char* FcgiConnHandler::FcgiTypeToName(int type)
{
    static const char* names[] = {
        "Unknown",
        "FCGI_BEGIN_REQUEST",       // 1
        "FCGI_ABORT_REQUEST",       // 2
        "FCGI_END_REQUEST",         // 3
        "FCGI_PARAMS",              // 4
        "FCGI_STDIN",               // 5
        "FCGI_STDOUT",              // 6
        "FCGI_STDERR",              // 7
        "FCGI_DATA",                // 8
        "FCGI_GET_VALUES",          // 9
        "FCGI_GET_VALUES_RESULT",   // 10
        "FCGI_UNKNOWN_TYPE",        // 11
    };

    if (type >= 0 && (unsigned)type < sizeof(names)/sizeof(names[0]))
        return names[type];
    else
        return names[0];
}

}}  // namespace fasmio::net_server

