
#include "./conn-base.h"
#include "./http-header-parser.h"
#include <assert.h>
#include <string.h>
#include <memory>
#include <sstream>
#include <string>

namespace fasmio { namespace net_client {

ConnBase::ConnBase(IRuntimeEnv* env, ILogger* logger, const char* module_name) :
    env_(env),
    logger_(logger),
    mlogger_(logger, module_name),
    sock_(nullptr)
{
}

ConnBase::~ConnBase()
{
    Close(nullptr);
}

bool ConnBase::Connect(const char* host_port)
{
    if (host_port == nullptr)
        return false;

    const char* colon = strchr(host_port, ':');
    if (colon == nullptr)
        return Connect(host_port, 0);

    int port = atoi(colon + 1);
    if (port <= 0 || port > 0xFFFF)
        return false;

    const std::string host(host_port, colon - host_port);
    return Connect(host.c_str(), port);
}

bool ConnBase::Connect(const char* host, unsigned short port)
{
    if (host == nullptr || port < 0 || port > 0xFFFF)
        return false;
    if (sock_ != nullptr)
        return false;

    if (port == 0)
        port = GetDefaultPort();

    if (nullptr == (sock_ = env_->NewTCPSocket()))
        return false;

    PLOG("Connecting %s:%d ...", host, port);
    if (!sock_->Connect(host, port))
    {
        delete sock_;
        sock_ = nullptr;
        return false;
    }

    PLOG("Connection to %s:%d established", host, port);
    return true;
}

bool ConnBase::SendRequest(const char* method, const char* url)
{
    return SendRequest(method, url, nullptr);
}

bool ConnBase::SendRequest(const char* method, const char* url, const char* data)
{
    const long content_length = (data != nullptr ? strlen(data) : 0);
    IOStream *os = StartRequest(method, url, content_length);
    if (os == nullptr)
        return false;

    if (data != nullptr && content_length > 0)
    {
        if (content_length != os->Write(data, content_length))
        {
            EndRequest(&os);
            OnError();
            return false;
        }
    }

    return EndRequest(&os);
}

IOStream* ConnBase::StartRequest(const char* method, const char* url)
{
    return StartRequest(method, url, kContentLengthNotSpecified);
}

bool ConnBase::Retrieve()
{
    return Retrieve(nullptr);
}

bool ConnBase::Retrieve(int *status_code)
{
    IIStream *is = nullptr;
    unsigned long content_length = 0;

    if (!Retrieve(status_code, &is, &content_length))
        return false;
    assert(is != nullptr);

    // read all the response and discard it
    while (true)
    {
        char buff[4096];
        long bytes = is->Read(buff, sizeof(buff));
        if (bytes <= 0)
            break;
    }

    Close(&is);
    return true;
}

bool ConnBase::Retrieve(int *status_code, IIStream **is, unsigned long *content_length)
{
    std::unique_ptr<HttpHeaderParser> parser(new HttpHeaderParser(static_cast<IIStream*>(this)));

    int code = 200;
    if (parser->GetStatus(&code))
        PLOG("Received: %d", code);
    else
        PLOG("HTTP status code not found, assumed 200");
    if (status_code != nullptr)
        *status_code = code;

    bool content_length_found = false;
    while (true)
    {
        const char *header_name = nullptr;
        const char *header_value = nullptr;
        if (!parser->GetNextHeader(&header_name, &header_value))
        {
            PLOG("HTTP parse header failed, probably it is too long");
            break;
        }

        if (header_name == nullptr && header_value == nullptr)
            break;

        PLOG("Received: %s: %s", header_name, header_value);
        if (content_length != nullptr && 0 == strcasecmp("Content-Length", header_name))
        {
            content_length_found = true;
            *content_length = atoi(header_value);
        }
    }

    if (!content_length_found)
    {
        PLOG("Header Content-Length not found!");
        if (content_length != nullptr)
            *content_length = kContentLengthNotSpecified;
    }

    if (is != nullptr)
        *is = static_cast<IIStream*>(parser.release());
    PLOG("Headers end.");

    return true;
}

bool ConnBase::Close(IIStream **is)
{
    if (is != nullptr && *is != nullptr)
    {
        delete *is;
        *is = nullptr;
    }

    if (sock_ != nullptr)
    {
        delete sock_;
        sock_ = nullptr;
    }

    return true;
}

void ConnBase::OnError()
{
    Close(nullptr);
}

}}  // namespace fasmio::net_client

