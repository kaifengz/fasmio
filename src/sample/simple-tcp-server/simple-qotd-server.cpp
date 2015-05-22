
#include "./qotd.h"
#include "runtime-env/fiber-env/fiber-env.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static bool quit_flag = false;

void die(const char* msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void on_sigint(int)
{
    quit_flag = true;
}

int main_proc(fasmio::IRuntimeEnv* env, int argc, char* argv[])
{
    signal(SIGINT, on_sigint);

    SimpleTCPServer server(env);

    if (!server.Start(1700, QotdConnHandler::CreateInstance))
        die("Failed to start the QOTD server");
    while (!quit_flag)
        usleep(1000*100);
    server.Stop();

    return 0;
}

int main(int argc, char* argv[])
{
    fasmio::fiber_env::FiberEnv env;
    return env.RunMain(main_proc, argc, argv);
}

