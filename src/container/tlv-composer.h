
#ifndef CONTAINER_TLV_COMPOSER_H_
#define CONTAINER_TLV_COMPOSER_H_

#include "interface/stream.h"
#include <istream>
#include <vector>

namespace fasmio { namespace container {

class TlvComposer : public IOStream
{
public:
    explicit TlvComposer(IOStream *os);
    ~TlvComposer();

public:
    bool CheckIntegrity() const;

    bool BeginKey(const char* name);
    bool EndKey(const char* name = nullptr);

    IOStream* BeginValue(const char* name);
    long EndValue(IOStream*);

    bool AddValue(const char* name, const char* value);
    bool AddValue(const char* name, const unsigned char* value, unsigned long length);
    bool AddValue(const char* name, long value);
    bool AddValue(const char* name, unsigned long value);
    bool AddValue(const char* name, double value);

    long GetWroteSize();

protected:
    // for IOStream
    virtual long Write(const void* buff, long size);

private:
    bool add_value(const char* name, const unsigned char* value, unsigned long length);
    bool put_var_int(long value);

private:
    IOStream *os_;
    std::vector<std::string> keys_;
    long total_wrote_;
    long curr_value_len_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_TLV_COMPOSER_H_

