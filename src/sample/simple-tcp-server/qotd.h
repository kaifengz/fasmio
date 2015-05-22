
#ifndef SIMPLE_QOTD_H
#define SIMPLE_QOTD_H

#include "tcp-server.h"

class QotdConnHandler : public ConnectionHandler
{
public:
    explicit QotdConnHandler(fasmio::IRuntimeEnv*);
    virtual ~QotdConnHandler();

public:
    static ConnectionHandler* CreateInstance(fasmio::IRuntimeEnv*);

public:
    virtual void HandleConnection(fasmio::runtime_env::ITCPSocket*, const char* addr);

private:
    fasmio::IRuntimeEnv* env_;
};

#endif  // SIMPLE_QOTD_H

