
#include "container/container-impl.h"
#include "common/ini-reader.h"
#include "common/logger.h"
#include "common/module-logger.h"
#include "runtime-env/fiber-env/fiber-env.h"
#include "runtime-env/pthread-env/pthread-env.h"
#include "net-client/fcgi-client.h"
#include "net-server/fcgi-server.h"

#include <signal.h>
#include <unistd.h>
#include <memory>

#define CONFIG_FILE         "config/fasmio.ini"

static bool quit_flag = false;

#if 0
typedef fasmio::fiber_env::FiberEnv         RuntimeEnv;
#else
typedef fasmio::pthread_env::PthreadEnv     RuntimeEnv;
#endif

typedef fasmio::net_client::FcgiClient      NetClient;
typedef fasmio::net_server::FcgiServer      NetServer;
typedef fasmio::container::ContainerImpl    Container;

void sigint_handler(int)
{
    quit_flag = true;
}

int real_main(fasmio::IRuntimeEnv* env, int argc, char* argv[])
{
    sighandler_t handler = signal(SIGINT, sigint_handler);
    signal(SIGPIPE, SIG_IGN);

    fasmio::common::Logger logger(env);
    fasmio::common::ModuleLogger mlogger(&logger, "main");
    mlogger.Info("Starting server ...");

    fasmio::common::IniReader ini_reader;
    if (!ini_reader.Read(CONFIG_FILE))
    {
        mlogger.Error("Configuration file %s not found!", CONFIG_FILE);
        return 1;
    }

    std::unique_ptr<fasmio::INetClient> net_client(new NetClient(env, &logger));

    std::unique_ptr<fasmio::IContainer> container(new Container(env, &logger));
    if (!container->Initialize(net_client.get(), &ini_reader))
    {
        mlogger.Error("Failed to initialize container");
        return 1;
    }

    std::unique_ptr<fasmio::INetServer> net_server(new NetServer(env, &logger));
    if (!net_server->Start(container.get(), &ini_reader))
    {
        mlogger.Error("Failed to start net-server");
        return 1;
    }

    mlogger.Info("Server started");

    while (!quit_flag)
        env->Sleep(10);
    signal(SIGINT, handler);

    mlogger.Info("Stopping server ...");

    net_server->Stop();
    net_server.reset();

    container->Finalize();
    container.reset();

    mlogger.Info("Server stopped");
    return 0;
}

int main(int argc, char* argv[])
{
    RuntimeEnv env;
    return env.RunMain(real_main, argc, argv);
}

