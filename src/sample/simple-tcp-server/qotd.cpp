
#include "qotd.h"
#include <string.h>

#define QUOTE "An apple a day, keeps doctor away."

QotdConnHandler::QotdConnHandler(fasmio::IRuntimeEnv* env) :
    env_(env)
{
}

QotdConnHandler::~QotdConnHandler()
{
}

ConnectionHandler* QotdConnHandler::CreateInstance(fasmio::IRuntimeEnv* env)
{
    return new QotdConnHandler(env);
}

void QotdConnHandler::HandleConnection(fasmio::runtime_env::ITCPSocket* sock, const char* addr)
{
    sock->SendAll(QUOTE, strlen(QUOTE));
//    sock->Shutdown(SHUT_RDWR);   // TODO:
}

