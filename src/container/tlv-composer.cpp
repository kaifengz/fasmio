
#include "./tlv-composer.h"
#include "./stream-impl.h"
#include "common/stream-adaptor.h"

#include <string.h>

namespace fasmio { namespace container {

TlvComposer::TlvComposer(IOStream *os) :
    os_(os),
    keys_(),
    total_wrote_(0),
    curr_value_len_(0)
{
}

TlvComposer::~TlvComposer()
{
}

bool TlvComposer::CheckIntegrity() const
{
    return os_ != nullptr && keys_.empty();
}

bool TlvComposer::BeginKey(const char* name)
{
    if (os_ == nullptr || name == nullptr)
        return false;

    long length = strlen(name);
    if (length == 0 || length >= 128)
        return false;

    if (1 != os_->Write("K", 1))
        return false;
    total_wrote_ += 1;
    if (!put_var_int(length))
        return false;
    if (length != os_->Write(name, length))
        return false;

    keys_.push_back(name);
    total_wrote_ += length;
    return true;
}

bool TlvComposer::EndKey(const char* name)
{
    if (os_ == nullptr)
        return false;

    if (keys_.empty())
        return false;

    if (name != nullptr)
    {
        if (keys_.back() != name)
            return false;
    }

    if (1 != os_->Write("E", 1))
        return false;

    keys_.pop_back();
    total_wrote_ += 1;
    return true;
}

IOStream* TlvComposer::BeginValue(const char* name)
{
    if (os_ == nullptr || name == nullptr)
        return nullptr;

    int name_len = strlen(name);
    if (name_len >= 128)
        return nullptr;

    if (1 != os_->Write("V", 1))
        return nullptr;
    total_wrote_ += 1;
    if (!put_var_int(name_len))
        return nullptr;
    if (name_len != os_->Write(name, name_len))
        return nullptr;
    total_wrote_ += name_len;

    curr_value_len_ = 0;
    return static_cast<IOStream*>(this);
}

long TlvComposer::Write(const void* buff, long size)
{
    if (os_ == nullptr || buff == nullptr)
        return -1;

    if (size > 0)
    {
        if (!put_var_int(size))
            return -1;
        if (size != os_->Write(buff, size))
            return -1;
        total_wrote_ += size;
        curr_value_len_ += size;
    }

    return size;
}

long TlvComposer::EndValue(IOStream* os)
{
    if (os_ == nullptr || os != static_cast<IOStream*>(this))
        return -1;

    if (!put_var_int(0))
        return -1;
    return curr_value_len_;
}

bool TlvComposer::AddValue(const char* name, const char* value)
{
    if (os_ == nullptr || name == nullptr || value == nullptr)
        return false;

    return add_value(name, reinterpret_cast<const unsigned char*>(value), strlen(value));
}

bool TlvComposer::AddValue(const char* name, const unsigned char* value, unsigned long length)
{
    if (os_ == nullptr || name == nullptr || value == nullptr)
        return false;

    return add_value(name, value, length);
}

bool TlvComposer::AddValue(const char* name, long value)
{
    if (os_ == nullptr || name == nullptr)
        return false;

    char buff[64];
    long length = snprintf(buff, sizeof(buff), "%ld", value);
    return add_value(name, reinterpret_cast<const unsigned char*>(buff), length);
}

bool TlvComposer::AddValue(const char* name, unsigned long value)
{
    if (os_ == nullptr || name == nullptr)
        return false;

    char buff[64];
    long length = snprintf(buff, sizeof(buff), "%lu", value);
    return add_value(name, reinterpret_cast<const unsigned char*>(buff), length);
}

bool TlvComposer::AddValue(const char* name, double value)
{
    if (os_ == nullptr || name == nullptr)
        return false;

    char buff[64];
    long length = snprintf(buff, sizeof(buff), "%f", value);
    return add_value(name, reinterpret_cast<const unsigned char*>(buff), length);
}

long TlvComposer::GetWroteSize()
{
    return total_wrote_;
}

bool TlvComposer::add_value(const char* name, const unsigned char* value, unsigned long length)
{
    int name_len = strlen(name);
    if (name_len >= 128)
        return false;

    if (1 != os_->Write("V", 1))
        return false;
    total_wrote_ += 1;
    if (!put_var_int(name_len))
        return false;
    if (name_len != os_->Write(name, name_len))
        return false;
    total_wrote_ += name_len;
    if (length > 0)
    {
        if (!put_var_int(length))
            return false;
        if ((long)length != os_->Write(value, length))
            return false;
        total_wrote_ += length;
    }
    if (!put_var_int(0))
        return false;
    return true;
}

bool TlvComposer::put_var_int(long value)
{
#ifdef TLV_ASCII_ONLY

    const char* hex_string = "0123456789ABCDEF";
    char hex[8];
    long length;

    if (value < 128)
    {
        hex[0] = hex_string[value >> 4];
        hex[1] = hex_string[value & 0xF];
        length = 2;
    }
    else
    {
        for (int i=0; i<8; ++i)
        {
            hex[7-i] = hex_string[ value & 0xF ];
            value >>= 4;
        }
        hex[0] |= 0x80;
        length = 8;
    }

    if (length != os_->Write(hex, length))
        return false;
    total_wrote_ += length;
    return true;

#else  // TLV_ASCII_ONLY

    char vint[4];
    long vlen;
    if (value < 128)
    {
        vint[0] = value;
        vlen = 1;
    }
    else
    {
        vint[0] = ((value >> 24) | 0x80);
        vint[1] = ((value >> 16) & 0xFF);
        vint[2] = ((value >>  8) & 0xFF);
        vint[3] = ( value        & 0xFF);
        vlen = 4;
    }

    if (vlen != os_->Write(vint, vlen))
        return false;
    total_wrote_ += vlen;
    return true;

#endif  // TLV_ASCII_ONLY
}

}}  // namespace fasmio::container

