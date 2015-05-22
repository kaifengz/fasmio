
#ifndef FTP_CONN_HANDLER_H
#define FTP_CONN_HANDLER_H

#include "tcp-server.h"

#include <string>

class FtpConnHandler : public ConnectionHandler
{
public:
    explicit FtpConnHandler(fasmio::IRuntimeEnv* env);
    virtual ~FtpConnHandler();

public:
    static ConnectionHandler* CreateInstance(fasmio::IRuntimeEnv* env);

public:
    virtual void HandleConnection(fasmio::runtime_env::ITCPSocket*, const char* addr);

public:
    enum FtpCommand
    {
        CMD_UNKNOWN = 0,
        CMD_USER,
        CMD_PASS,
        CMD_NOOP,
        CMD_QUIT,
        CMD_PWD,
        CMD_CWD,
        CMD_TYPE,
        CMD_PORT,
        CMD_PASV,
        CMD_LIST,
        CMD_RETR,
    };

    bool On_USER();
    bool On_PASS();
    bool On_NOOP();
    bool On_UNKNOWN();
    bool On_PWD();
    bool On_CWD();
    bool On_LIST();
    bool On_RETR();
    bool On_TYPE();
    bool On_PORT();

private:
    bool SendMessage(int code, const char* msg, ...);
    bool WaitCommand();

    bool OpenDataConnection();
    void CloseDataConnection();

    static bool NormalizePath(std::string &);
    static bool IsDirectory(const std::string &);
    static bool IsFile(const std::string &);
    static bool ListDirectory(const std::string &dir, fasmio::runtime_env::ITCPSocket* sock);
    static bool SendFile(const std::string &path, bool bin_mode, fasmio::runtime_env::ITCPSocket* sock);
    static bool FileToSocket(FILE*, fasmio::runtime_env::ITCPSocket* sock);

private:
    fasmio::IRuntimeEnv *env_;
    fasmio::runtime_env::ITCPSocket *sock_;
    char buffer_[1024];
    FtpCommand last_cmd_;
    char* last_cmd_arg_;

    std::string working_dir_;
    bool bin_mode_;
    std::string data_channel_ip_;
    unsigned short data_channel_port_;
    fasmio::runtime_env::ITCPSocket *sock_data_;
};

#endif  // FTP_CONN_HANDLER_H

