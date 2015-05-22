
#ifndef COMMON_INI_READER_H_
#define COMMON_INI_READER_H_

#include "interface/config.h"
#include <istream>
#include <map>
#include <string>
#include <vector>

namespace fasmio { namespace common {

class IniReader : public IConfig
{
public:
    IniReader();
    virtual ~IniReader();

public:
    bool Read(const char* file_name);
    bool Read(std::istream &);

    const char* GetStrValue(const char* section, const char* option, const char* def_value);
    const char* GetStrValue(const std::string &section, const std::string &option, const char* def_value);

    long GetIntValue(const char* section, const char* option, long def_value);
    long GetIntValue(const std::string &section, const std::string &option, long def_value);

    void GetSections(std::vector<std::string> &sections);

    bool GetOptions(const char* section, std::vector<std::string> &options);
    bool GetOptions(const std::string &section, std::vector<std::string> &options);

private:
    struct string_caseless_comparer
    {
        bool operator() (const std::string &, const char* ) const;
        bool operator() (const std::string &, const std::string &) const;
    };

private:
    typedef std::map<std::string, std::string, string_caseless_comparer> options_t;
    typedef std::map<std::string, options_t, string_caseless_comparer> sections_t;
    sections_t sections_;
    bool read_;
};

}}  // namespace fasmio::common

#endif  // COMMON_INI_READER_H_

