
#include "./tlv-parser.h"
#include "common/ranged-istream.h"
#include <assert.h>

namespace fasmio { namespace container {

TlvParser::TlvParser(IIStream *is) :
    callbacks_stack(1),
    is_(is),
    sbuff_(),
    dbuff_(),
    buff_(sbuff_),
    buff_len_(sizeof(sbuff_)),
    value_to_read_(0)
{
}

TlvParser::~TlvParser()
{
}

void TlvParser::SetEnterKeyCallback(EnterKeyCallback enter_key_callback, void *arg)
{
    assert(!callbacks_stack.empty());
    callbacks_stack.back().enter_key_callback_ = enter_key_callback;
    callbacks_stack.back().enter_key_callback_arg_ = arg;
}

void TlvParser::SetLeaveKeyCallback(LeaveKeyCallback leave_key_callback, void *arg)
{
    assert(!callbacks_stack.empty());
    callbacks_stack.back().leave_key_callback_ = leave_key_callback;
    callbacks_stack.back().leave_key_callback_arg_ = arg;
}

void TlvParser::SetValueCallback(ValueCallback_istream value_callback, void *arg)
{
    assert(!callbacks_stack.empty());
    callbacks_stack.back().value_callback_istream_ = value_callback;
    callbacks_stack.back().value_callback_istream_arg_ = arg;
}

void TlvParser::SetValueCallback(ValueCallback_string value_callback, void *arg)
{
    assert(!callbacks_stack.empty());
    callbacks_stack.back().value_callback_string_ = value_callback;
    callbacks_stack.back().value_callback_string_arg_ = arg;
}

bool TlvParser::PushCallbacks()
{
    callbacks_stack.resize(callbacks_stack.size() + 1);
    return true;
}

bool TlvParser::RestoreCallbacks()
{
    if (callbacks_stack.size() <= 1)
        return false;

    callbacks_stack.pop_back();
    return true;
}

bool TlvParser::Parse()
{
    if (is_ == nullptr)
        return false;

    bool key_end = false;
    bool succeed = ParseNode(&key_end);
    return succeed && !key_end;
}

bool TlvParser::ParseNode(bool *is_key_end)
{
    // type: one byte, 'k', 'v', or 'e'
    char type;
    if (1 != is_->Read(&type, 1))
        return false;

    {   const bool key_end = (type == 'e' || type == 'E');
        if (is_key_end != nullptr)
            *is_key_end = key_end;
        if (key_end)
            return true;
    }

    const bool is_key = (type == 'k' || type == 'K');
    const bool is_value = (type == 'v' || type == 'V');

    if (!is_key && !is_value)
        return false;

    // name-len: 1~127
    const int name_len = GetVarInt();
    if (name_len <= 0 || name_len >= 128)
        return false;

    // name
    char name[128];
    if (name_len != is_->Read(name, name_len))
        return false;
    name[name_len] = '\0';

    // key or value
    if (is_key)
    {
        if (!ParseKey(name))
            return false;
    }
    else  // if (is_value)
    {
        if (!ParseValue(name))
            return false;
    }

    return true;
}

bool TlvParser::ParseKey(const char* name)
{
    assert(!callbacks_stack.empty());

    EnterKeyCallback enter_key = callbacks_stack.back().enter_key_callback_;
    if (enter_key != nullptr && !enter_key(name, callbacks_stack.back().enter_key_callback_arg_))
        return false;

    while (true)
    {
        bool key_end = false;
        if (!ParseNode(&key_end))
            return false;
        if (key_end)
            break;
    }

    LeaveKeyCallback leave_key = callbacks_stack.back().leave_key_callback_;
    if (leave_key != nullptr && !leave_key(name, callbacks_stack.back().leave_key_callback_arg_))
        return false;

    return true;
}

bool TlvParser::ParseValue(const char* name)
{
    assert(!callbacks_stack.empty());

    ValueCallback_istream value_istream = callbacks_stack.back().value_callback_istream_;
    ValueCallback_string  value_string  = callbacks_stack.back().value_callback_string_;
    if (value_istream != nullptr || value_string == nullptr)
    {
        IIStream* is = static_cast<IIStream*>(this);
        value_to_read_ = 0;

        if (value_istream != nullptr)
        {
            if (!value_istream(name, is, callbacks_stack.back().value_callback_istream_arg_))
                return false;
        }

        // drain out the value
        long bytes;
        while ((bytes = is->Read(buff_, buff_len_)) > 0)
            ;
        return bytes >= 0;
    }
    else // if (value_string != nullptr)
    {
        long value_len = 0;
        while (true)
        {
            long part_len = GetVarInt();
            if (part_len == 0)
                break;
            if (value_len + part_len > (long)buff_len_)
            {
                buff_len_ = std::max((long)(buff_len_ * 3 / 2), value_len + part_len);
                dbuff_.resize(buff_len_);
                buff_ = &dbuff_[0];
            }
            if (part_len != is_->Read(buff_ + value_len, part_len))
                return false;
            value_len += part_len;
        }

        return value_string(name, buff_, value_len, callbacks_stack.back().value_callback_string_arg_);
    }
}

long TlvParser::Read(void* buff, long size)
{
    if (value_to_read_ < 0)
        return 0;

    long copied = 0;
    while (true)
    {
        if (value_to_read_ == 0)
        {
            if ((value_to_read_ = GetVarInt()) == 0)
            {
                value_to_read_ = -1;
                break;
            }
        }

        long to_read = std::min(value_to_read_, size);
        if (to_read != is_->Read(buff, to_read))
            return -1;
        copied += to_read;
        value_to_read_ -= to_read;
        if (0 == (size -= to_read))
            break;
        buff = ((char*)buff) + to_read;
    }
    return copied;
}

int TlvParser::GetVarInt()
{
#ifdef TLV_ASCII_ONLY
#   define PARSE_HEX(h)    \
    ((h >= '0' && h <= '9') ? h - '0' : \
    (h >= 'a' && h <= 'f') ? h - 'a' + 10 : \
    (h >= 'A' && h <= 'F') ? h - 'A' + 10 : -1)

    int n = 0;
    int tmp;
    char hex[8];
    if (2 != is_->Read(hex, 2))
        return -1;

    if ((tmp = PARSE_HEX(hex[0])) < 0)
        return -1;
    n += (tmp << 4);
    if ((tmp = PARSE_HEX(hex[1])) < 0)
        return -1;
    n += tmp;

    if (n >= 128)
    {
        n -= 128;

        if (6 != is_->Read(hex, 6))
            return -1;

        for (int i=0; i<6; ++i)
        {
            if ((tmp = PARSE_HEX(hex[i])) < 0)
                return -1;
            n = ((n << 4) | tmp);
        }
    }

    return n;

#else  // TLV_ASCII_ONLY

    unsigned char vint[4];
    if (1 != is_->Read(vint, 1))
        return -1;

    if (vint[0] <= 0x7F)
        return vint[0];

    if (3 != is_->Read(vint+1, 3))
        return -1;

    return ((vint[0] & 0x7F) << 24) | (vint[1] << 16) | (vint[2] << 8) | vint[3];

#endif  // TLV_ASCII_ONLY
}

TlvParser::Callbacks::Callbacks() :
    enter_key_callback_(nullptr),
    enter_key_callback_arg_(nullptr),
    leave_key_callback_(nullptr),
    leave_key_callback_arg_(nullptr),
    value_callback_istream_(nullptr),
    value_callback_istream_arg_(nullptr),
    value_callback_string_(nullptr),
    value_callback_string_arg_(nullptr)
{
}

}}  // namespace fasmio::container

