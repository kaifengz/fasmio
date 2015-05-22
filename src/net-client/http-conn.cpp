
#include "./http-conn.h"
#include <assert.h>
#include <string.h>
#include <sstream>
#include <string>

namespace fasmio { namespace net_client {

const unsigned short kDefaultHttpPort = 80;

HttpConn::HttpConn(IRuntimeEnv* env, ILogger* logger) :
    ConnBase(env, logger, "HttpClient")
{
}

HttpConn::~HttpConn()
{
}

IOStream* HttpConn::StartRequest(const char* method, const char* url, unsigned long content_length)
{
    if (method == nullptr || url == nullptr)
        return nullptr;
    if (sock_ == nullptr)
        return nullptr;

    std::ostringstream oss;

    oss << method << " " << url << " HTTP/1.0\r\n";
    if (content_length != kContentLengthNotSpecified)
        oss << "Content-Length: " << content_length << "\r\n";
    oss << "\r\n";

    const std::string header = oss.str();
    if ((long)header.size() != sock_->SendAll(header.c_str(), header.size()))
    {
        PLOG("Failed to send the request header (%d bytes)", header.size());
        OnError();
        return nullptr;
    }

    PLOG("Sent: %s %s HTTP/1.0", method, url);
    if (content_length != kContentLengthNotSpecified)
        PLOG("Sent: Content-Length: %d", content_length);
    PLOG("Sent: ");

    return static_cast<IOStream*>(this);
}

long HttpConn::Write(const void* buff, long size)
{
    if (buff == nullptr || size <= 0)
        return 0;
    if (sock_ == nullptr)
        return -1;

    long sent = sock_->Send(buff, size);
    if (sent > 0)
        PLOG("Sent: %.*s", sent, buff);
    return sent;
}

bool HttpConn::EndRequest(IOStream **os)
{
    if (os != nullptr)
        *os = nullptr;
    if (sock_ == nullptr)
        return false;

    if (!sock_->Shutdown(runtime_env::ITCPSocket::SHUT_SEND))
    {
        PLOG("Failed to shutdown(SHUT_SEND)");
        OnError();
        return false;
    }

    PLOG("Request done");
    return true;
}

long HttpConn::Read(void* buff, long size)
{
    if (buff == nullptr || size <= 0)
        return 0;
    if (sock_ == nullptr)
        return -1;

    long received = sock_->Receive(buff, size);
    if (received > 0)
        PLOG("Received: %.*s", received, buff);
    return received;
}

unsigned short HttpConn::GetDefaultPort()
{
    return kDefaultHttpPort;
}

}}  // namespace fasmio::net_client

