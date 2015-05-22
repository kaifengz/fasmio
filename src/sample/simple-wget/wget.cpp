
#include "runtime-env/fiber-env/fiber-env.h"
#include "net-client/http-client.h"
#include "common/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>

bool parse_url(const char* full_url, std::string &host, unsigned short &port)
{
    if (0 != strncasecmp("http://", full_url, 7))
        return false;
    const char* begin_of_host = full_url + 7;

    const char* slash = strchr(begin_of_host, '/');
    if (slash == nullptr)
        return false;

    const char* end_of_host = slash;
    const char* colon = strchr(begin_of_host, ':');
    if (colon == nullptr || colon > slash)
        port = 80;
    else
    {
        long n = atoi(colon + 1);
        if (n <= 0 || n > 0xFFFF)
            return false;

        port = n;
        end_of_host = colon;
    }

    if (end_of_host == begin_of_host)
        return false;

    host = std::string(begin_of_host, end_of_host - begin_of_host);
    return true;
}

void die(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(2);
}

int wget(fasmio::IRuntimeEnv* env, const char* full_url)
{
    std::string host;
    unsigned short port;

    if (!parse_url(full_url, host, port))
        die("Invalid URL: %s", full_url);

    std::unique_ptr<fasmio::ILogger> logger(new fasmio::common::Logger(env));
    std::unique_ptr<fasmio::INetClient> net_client(new fasmio::net_client::HttpClient(env, logger.get()));
    std::unique_ptr<fasmio::INetClientConnection> conn(net_client->NewConnection());

    if (!conn->Connect(host.c_str(), port))
        die("Cannot connect to %s:%d", host.c_str(), port);

    if (!conn->SendRequest("GET", full_url))
        die("Failed to send HTTP request to %s:%d", host.c_str(), port);

    int status_code = 0;
    fasmio::IIStream *is = nullptr;
    unsigned long content_length = 0;
    if (!conn->Retrieve(&status_code, &is, &content_length))
        die("Failed to retrieve the HTTP response from %s:%d", host.c_str(), port);
    if (status_code != 200)
        die("Statue code %d", status_code);
    if (is == nullptr)
        die("Unexpected program error");

    while (true)
    {
        char buff[4096];
        long bytes = is->Read(buff, sizeof(buff));
        if (bytes <= 0)
            break;
        fwrite(buff, 1, bytes, stdout);
    }

    conn->Close(&is);
    fflush(stdout);

    return 0;
}

void usage(const char* my_name)
{
    fprintf(stderr, "Usage: %s URL\n", my_name);
    exit(1);
}

int main_proc(fasmio::IRuntimeEnv* env, int argc, char* argv[])
{
    if (argc != 2)
        usage(argv[0]);

    return wget(env, argv[1]);
}

int main(int argc, char* argv[])
{
    fasmio::fiber_env::FiberEnv env;
    return env.RunMain(main_proc, argc, argv);
}

