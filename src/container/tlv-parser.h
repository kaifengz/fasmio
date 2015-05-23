
#ifndef CONTAINER_TLV_PARSER_H_
#define CONTAINER_TLV_PARSER_H_

#include "interface/stream.h"
#include <vector>

namespace fasmio { namespace container {

class TlvParser : IIStream
{
public:
    explicit TlvParser(IIStream *is);
    ~TlvParser();

public:
    typedef bool (*EnterKeyCallback)(const char* name, void* arg);
    void SetEnterKeyCallback(EnterKeyCallback enter_key_callback, void* arg);

    typedef bool (*LeaveKeyCallback)(const char* name, void* arg);
    void SetLeaveKeyCallback(LeaveKeyCallback leave_key_callback, void* arg);

    typedef bool (*ValueCallback_istream)(const char* name, IIStream *value, void* arg);
    void SetValueCallback(ValueCallback_istream value_callback, void* arg);

    typedef bool (*ValueCallback_string)(const char* name, const char* value, unsigned int value_len, void* arg);
    void SetValueCallback(ValueCallback_string value_callback, void* arg);

    bool PushCallbacks();
    bool RestoreCallbacks();

    enum ParseResult
    {
        R_FAILED = 0,
        R_KEY,
        R_VALUE,
        R_KEY_END,
    };

    ParseResult Parse();

protected:
    // for IIStream
    virtual long Read(void* buff, long size);

private:
    ParseResult ParseNode();
    bool ParseKey(const char* name);
    bool ParseValue(const char* name);
    int GetVarInt();

private:
    struct Callbacks
    {
        EnterKeyCallback       enter_key_callback_;
        void                  *enter_key_callback_arg_;
        LeaveKeyCallback       leave_key_callback_;
        void                  *leave_key_callback_arg_;
        ValueCallback_istream  value_callback_istream_;
        void                  *value_callback_istream_arg_;
        ValueCallback_string   value_callback_string_;
        void                  *value_callback_string_arg_;

        Callbacks();
    };

    std::vector<Callbacks>     callbacks_stack;    
    IIStream                  *is_;
    char                       sbuff_[512];
    std::vector<char>          dbuff_;
    char                      *buff_;
    unsigned long              buff_len_;
    long                       value_to_read_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_TLV_PARSER_H_

