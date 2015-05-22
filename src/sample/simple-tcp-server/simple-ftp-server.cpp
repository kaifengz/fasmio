
#include "./ftp.h"
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

    // TODO, FIXME: still can't quit as the listen thread
    // of the tcp-server is blocking on Accept.  Two possible
    // solutions:
    //   1. add a timeout to Accept, which is irreasonable
    //   2. force all socket operation fail during fiber-env
    //      shuting down
    //
}

int main_proc(fasmio::IRuntimeEnv* env, int argc, char* argv[])
{
    signal(SIGINT, on_sigint);

    SimpleTCPServer server(env);

    if (!server.Start(2100, FtpConnHandler::CreateInstance))
        die("Failed to start the FTP server");
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

