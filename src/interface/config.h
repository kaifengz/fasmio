
#ifndef INTERFACE_CONFIG_H_
#define INTERFACE_CONFIG_H_

namespace fasmio {

class IConfig
{
public:
    virtual ~IConfig() {}

public:
    virtual const char* GetStrValue(const char* section, const char* option, const char* def_value) = 0;

    virtual long GetIntValue(const char* section, const char* option, long def_value) = 0;
};

}  // namespace fasmio

#endif  // INTERFACE_CONFIG_H_

