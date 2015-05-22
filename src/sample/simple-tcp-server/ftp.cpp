
#include "ftp.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

// #define ENABLE_LOG
#ifdef ENABLE_LOG
#   define LOG(msg) fprintf(stderr, "%s", msg)
#else
#   define LOG(msg) ((void)0)
#endif

static const struct
{
    FtpConnHandler::FtpCommand cmd;
    const char* cmd_str;
    bool (FtpConnHandler::*cmd_handler)();
} commands[] =
{
    {FtpConnHandler::CMD_USER,   "USER",  &FtpConnHandler::On_USER},
    {FtpConnHandler::CMD_PASS,   "PASS",  &FtpConnHandler::On_PASS},
    {FtpConnHandler::CMD_QUIT,   "QUIT",  NULL},
    {FtpConnHandler::CMD_NOOP,   "NOOP",  &FtpConnHandler::On_NOOP},
    {FtpConnHandler::CMD_PWD ,   "PWD",   &FtpConnHandler::On_PWD},
    {FtpConnHandler::CMD_CWD ,   "CWD",   &FtpConnHandler::On_CWD},
    {FtpConnHandler::CMD_LIST,   "LIST",  &FtpConnHandler::On_LIST},
    {FtpConnHandler::CMD_RETR,   "RETR",  &FtpConnHandler::On_RETR},
    {FtpConnHandler::CMD_TYPE,   "TYPE",  &FtpConnHandler::On_TYPE},
    {FtpConnHandler::CMD_PORT,   "PORT",  &FtpConnHandler::On_PORT},
    {FtpConnHandler::CMD_UNKNOWN, NULL,   &FtpConnHandler::On_UNKNOWN},
};

FtpConnHandler::FtpConnHandler(fasmio::IRuntimeEnv* env) :
    env_(env),
    sock_(NULL),
    buffer_(),
    last_cmd_(CMD_UNKNOWN),
    last_cmd_arg_(NULL),
    working_dir_("/"),
    bin_mode_(false),  // according to RFC, ASCII mode is the default one
    data_channel_ip_(),
    data_channel_port_(0),
    sock_data_(NULL)
{
}

FtpConnHandler::~FtpConnHandler()
{
    CloseDataConnection();
}

ConnectionHandler* FtpConnHandler::CreateInstance(fasmio::IRuntimeEnv* env)
{
    return new FtpConnHandler(env);
}

void FtpConnHandler::HandleConnection(fasmio::runtime_env::ITCPSocket* sock, const char* addr)
{
    if (NULL == (sock_ = sock))
        return;

    if (!SendMessage(220, "Greeting %s", addr))
        return;

    // wait for USER, user name ignored
    while (true)
    {
        if (!WaitCommand())
            return;
        if (last_cmd_ != CMD_USER)
        {
            if (!SendMessage(530, "Please login with USER and PASS."))
                return;
            continue;
        }
        break;
    }

    if (!SendMessage(331, "Please specify the password."))
        return;

    // wait for PASS, the password will dropped immediately
    while (true)
    {
        if (!WaitCommand())
            return;
        if (last_cmd_ != CMD_PASS)
        {
            if (!SendMessage(530, "Please login with USER and PASS."))
                return;
            continue;
        }
        break;
    }

    if (!SendMessage(230, "Login successful.")) return;

    // command main loop
    while (true)
    {
        if (!WaitCommand())
            return;

        if (last_cmd_ == CMD_QUIT)
        {
            SendMessage(221, "Goodbye");
            break;
        }

        for (unsigned i=0; i<sizeof(commands)/sizeof(commands[0]); ++i)
        {
            if (commands[i].cmd == last_cmd_)
            {
                if (!((this->*(commands[i].cmd_handler))()))
                    return;
                break;
            }
        }
    }
}

bool FtpConnHandler::SendMessage(int code, const char* msg, ...)
{
    char buff[256];

    snprintf(buff, sizeof(buff), "%d", code);
    buff[3] = ' ';

    va_list args;
    va_start(args, msg);
    vsnprintf(buff+4, sizeof(buff)-4, msg, args);
    va_end(args);
    buff[sizeof(buff)-2] = '\0';

    int msg_len = strlen(buff);
    buff[msg_len] = '\n';
    buff[++msg_len] = '\0';
    LOG(buff);

    if (msg_len != sock_->SendAll(buff, msg_len))
        return false;

    return true;
}

bool FtpConnHandler::WaitCommand()
{
    int received_len = 0;

    while (true)
    {
        int received = sock_->Receive(buffer_+received_len, sizeof(buffer_)-received_len);
        if (received <= 0)
            return false;

        char* cr = (char*)memchr(buffer_+received_len, '\n', received);
        char* lf = (char*)memchr(buffer_+received_len, '\r', received);

        if (cr != NULL || lf != NULL)
        {
            if (cr) *cr = '\0';
            if (lf) *lf = '\0';
            LOG(buffer_);
            LOG("\n");
            break;
        }

        received_len += received;
        if ((unsigned)received_len >= sizeof(buffer_))
            // line length exceed buffer size
            return false;
    }

    for (unsigned i=0; i<sizeof(commands)/sizeof(commands[0]); ++i)
    {
        const char* cmd_str = commands[i].cmd_str;
        if (cmd_str == NULL)
            continue;

        const int cmd_str_len = strlen(cmd_str);
        if (0 == strncasecmp(buffer_, cmd_str, cmd_str_len) &&
            (buffer_[cmd_str_len] == '\0' || buffer_[cmd_str_len] == ' '))
        {
            last_cmd_ = commands[i].cmd;
            for (last_cmd_arg_ = buffer_ + cmd_str_len; *last_cmd_arg_ == ' '; ++last_cmd_arg_)
                ;
            return true;
        }
    }

    last_cmd_ = CMD_UNKNOWN;
    last_cmd_arg_ = buffer_;
    return true;
}

bool FtpConnHandler::On_USER()
{
    return SendMessage(530, "Can't change from guest user.");
}

bool FtpConnHandler::On_PASS()
{
    return SendMessage(230, "Already logged in.");
}

bool FtpConnHandler::On_NOOP()
{
    return SendMessage(200, "NOOP ok.");
}

bool FtpConnHandler::On_PWD()
{
    return SendMessage(257, "%s", working_dir_.c_str());
}

bool FtpConnHandler::On_CWD()
{
    if (*last_cmd_arg_ == '\0')
        return SendMessage(550, "No path specified.");

    std::string new_working_dir;
    if (*last_cmd_arg_ == '/')
        new_working_dir = last_cmd_arg_;
    else
        new_working_dir = working_dir_ + "/" + last_cmd_arg_;

    if (!NormalizePath(new_working_dir))
        return SendMessage(550, "File unavailable.");

    if (!IsDirectory("." + new_working_dir))
        return SendMessage(550, "Not a directory.");

    working_dir_ = new_working_dir;
    return SendMessage(250, "%s", working_dir_.c_str());
}

bool FtpConnHandler::On_TYPE()
{
    if (last_cmd_arg_ != NULL && *(last_cmd_arg_+1) == '\0' &&
        (*last_cmd_arg_ == 'A' || *last_cmd_arg_ == 'a' ||
         *last_cmd_arg_ == 'I' || *last_cmd_arg_ == 'i'))
    {
        if (*last_cmd_arg_ == 'A' || *last_cmd_arg_ == 'a')
        {
            bin_mode_ = false;
            return SendMessage(200, "Switching to ASCII mode.");
        }
        else
        {
            bin_mode_ = true;
            return SendMessage(200, "Switching to Binary mode.");
        }
    }

    return SendMessage(504, "Unrecognized TYPE command.");
}

bool FtpConnHandler::On_PORT()
{
    int d[6];
    if (6 != sscanf(last_cmd_arg_, "%d,%d,%d,%d,%d,%d", d+0, d+1, d+2, d+3, d+4, d+5))
        return SendMessage(501, "Illegal PORT command.");

    for (int i=0; i<6; ++i)
        if (d[i] < 0 || d[i] >= 256)
            return SendMessage(501, "Illegal PORT command.");

    char ip[64];
    snprintf(ip, sizeof(ip), "%d.%d.%d.%d", d[0], d[1], d[2], d[3]);
    data_channel_ip_ = ip;
    data_channel_port_ = d[4] * 256 + d[5];
    return SendMessage(200, "PORT %s:%d ok.", data_channel_ip_.c_str(), data_channel_port_);
}

bool FtpConnHandler::On_LIST()
{
    if (data_channel_ip_.empty())
        return SendMessage(425, "Use PORT first.");

    std::string path(working_dir_);
    if (*last_cmd_arg_ != '\0')
    {
        if (*last_cmd_arg_ == '/')
            path = last_cmd_arg_;
        else
            path = working_dir_ + "/" + last_cmd_arg_;
        if (!NormalizePath(path))
            return SendMessage(550, "File unavailable.");
        if (!IsDirectory("." + path))
            return SendMessage(550, "Not a directory.");
    }

    if (!OpenDataConnection())
        return SendMessage(425, "Can't open data connection.");
    if (!SendMessage(150, "Here comes the directory listing."))
        return false;

    bool succeed = ListDirectory("." + path, sock_data_);
    CloseDataConnection();

    if (!succeed)
        return SendMessage(451, "Requested action aborted: local error in processing.");

    return SendMessage(226, "Directory send OK.");
}

bool FtpConnHandler::On_RETR()
{
    if (*last_cmd_arg_ == '\0')
        return SendMessage(550, "No file specified.");

    std::string path;
    if (*last_cmd_arg_ == '/')
        path = last_cmd_arg_;
    else
        path = working_dir_ + "/" + last_cmd_arg_;

    if (!NormalizePath(path))
        return SendMessage(550, "File unavailable.");
    if (!IsFile("." + path))
        return SendMessage(550, "Not a file.");

    if (!OpenDataConnection())
        return SendMessage(425, "Can't open data connection.");
    if (!SendMessage(150, "Here comes the file content."))
        return false;

    bool succeed = SendFile("." + path, bin_mode_, sock_data_);
    CloseDataConnection();

    if (!succeed)
        return SendMessage(451, "Requested action aborted: local error in processing.");

    return SendMessage(226, "File send OK.");
}

bool FtpConnHandler::On_UNKNOWN()
{
    return SendMessage(500, "Unknown command. [%s]", last_cmd_arg_);
}

bool FtpConnHandler::NormalizePath(std::string &path)
{
    assert(path.size() > 0);
    path = path + '/';  // add a trailing slash to simplify processing

    static const char* patterns[] =
    {
        "//", "/./",
    };
    for (unsigned i=0; i<sizeof(patterns)/sizeof(patterns[0]); ++i)
    {
        const size_t ptn_len = strlen(patterns[i]);
        size_t pos = 0;
        while (true)
        {
            pos = path.find(patterns[i], pos);
            if (pos == std::string::npos)
                break;
            path = path.replace(pos, ptn_len, "/");
        }
    }

    while (true)
    {
        size_t pos = path.find("/../", 0);
        if (pos == 0)
            // want to go to the parent of root?
            return false;
        if (pos == std::string::npos)
            break;

        size_t slash = path.rfind('/', pos-1);
        assert(slash != std::string::npos);
        path = path.replace(slash, pos+4-slash, "/");
    }

    assert(path.size() > 0);
    if (path.size() == 1)
        assert(path[0] == '/');
    else
    {
        assert(path[path.size()-1] == '/');
        path = path.substr(0, path.size()-1);
        assert(path[path.size()-1] != '/');
    }
    return true;
}

bool FtpConnHandler::IsDirectory(const std::string &path)
{
    struct stat st;
    if (0 != stat(path.c_str(), &st))
        return false;

    return S_ISDIR(st.st_mode);
}

bool FtpConnHandler::IsFile(const std::string &path)
{
    struct stat st;
    if (0 != stat(path.c_str(), &st))
        return false;

    return S_ISREG(st.st_mode);
}

bool FtpConnHandler::ListDirectory(const std::string &dir, fasmio::runtime_env::ITCPSocket* sock)
{
    char buff[256];
    snprintf(buff, sizeof(buff), "ls -o -g '%s' | tail -n +2 | unix2dos", dir.c_str());  // FIXME, security issue here
    FILE* fp = popen(buff, "r");
    if (fp == NULL)
        return false;

    bool succeed = FileToSocket(fp, sock);
    pclose(fp);
    return succeed;
}

bool FtpConnHandler::SendFile(const std::string &path, bool bin_mode, fasmio::runtime_env::ITCPSocket* sock)
{
    FILE* fp = fopen(path.c_str(), bin_mode ? "rb" : "r");
    if (fp == NULL)
        return false;

    bool succeed = FileToSocket(fp, sock);
    fclose(fp);
    return succeed;
}

bool FtpConnHandler::FileToSocket(FILE *fp, fasmio::runtime_env::ITCPSocket *sock)
{
    while (true)
    {
        char buff[4096];
        int read = fread(buff, 1, sizeof(buff), fp);
        if (read < 0)
            return false;
        if (read == 0)
            return true;
        if (!sock->SendAll(buff, read))
            return false;
    }
}

bool FtpConnHandler::OpenDataConnection()
{
    CloseDataConnection();

    sock_data_ = env_->NewTCPSocket();
    if (sock_data_ == NULL)
        return false;

    if (!sock_data_->Connect(data_channel_ip_.c_str(), data_channel_port_))
        return false;

    return true;
}

void FtpConnHandler::CloseDataConnection()
{
    if (sock_data_ != NULL)
    {
        sock_data_->Close();
        delete sock_data_;
        sock_data_ = NULL;
    }
}

