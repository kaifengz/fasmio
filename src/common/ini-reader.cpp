
#include "./ini-reader.h"
#include <fstream>
#include <string.h>

namespace fasmio { namespace common {

IniReader::IniReader() :
    sections_(),
    read_(false)
{
}

IniReader::~IniReader()
{
}

bool IniReader::Read(const char* file_name)
{
    if (read_)
        return false;
    if (file_name == nullptr)
        return false;

    std::ifstream fin(file_name, std::ios_base::in);
    if (!fin)
        return false;

    bool result = Read(fin);
    fin.close();
    return result;
}

bool IniReader::Read(std::istream &is)
{
    if (read_)
        return false;

    std::string section;
    sections_t::iterator iter;

    while (true)
    {
        char line[1024];
        is.getline(line, sizeof(line));
        if (is.gcount() <= 0)
            break;

        const char *p = line;

        // skip leading white space
        while (*p == ' ' || *p == '\t')
            ++p;

        if (*p == '\0' || *p == ';' || *p == '#')
            // empty line or comment line
            continue;

        if (*p == '[')
        {
            do ++p;
            while (*p == ' ' || *p == '\t');

            const char *q = strchr(p, ']');
            if (q == nullptr)
                // malformed section line, ignore
                continue;

            while (*(q-1) == ' ' || *(q-1) == '\t')
                --q;
            if (p >= q)
                // empty section name, ignore
                continue;

            section.assign(p, q-p);
            sections_[section];
            iter = sections_.find(section);
        }
        else if (section.empty())
            // ignore every line if section is not specified
            continue;
        else
        {
            const char* equation = strchr(p, '=');
            if (equation == nullptr)
                // not an option line, ignore
                continue;
            if (p == equation)
                // empty option name, ignore
                continue;

            const char* q = equation;
            while (*(q-1) == ' ' || *(q-1) == '\t')
                --q;

            std::string option(p, q-p);
            std::string value;

            const char* v = equation+1;
            while (*v == ' ' || *v == '\t')
                ++v;
            if (*v != '\0')
            {
                const char* ve = v + strlen(v);
                while (*(ve-1) == ' ' || *(ve-1) == '\t')
                    --ve;
                value.assign(v, ve-v);
            }

            iter->second.insert(std::make_pair(option, value));
        }
    }

    return true;
}

const char* IniReader::GetStrValue(const char* section, const char* option, const char* def_value)
{
    return GetStrValue(std::string(section), std::string(option), def_value);
}

const char* IniReader::GetStrValue(const std::string &section, const std::string &option, const char* def_value)
{
    sections_t::const_iterator iter = sections_.find(section);
    if (iter == sections_.end())
        return def_value;

    options_t::const_iterator iter2 = iter->second.find(option);
    if (iter2 == iter->second.end())
        return def_value;

    return iter2->second.c_str();
}

long IniReader::GetIntValue(const char* section, const char* option, long def_value)
{
    return GetIntValue(std::string(section), std::string(option), def_value);
}

long IniReader::GetIntValue(const std::string &section, const std::string &option, long def_value)
{
    const char* value = GetStrValue(section, option, nullptr);
    if (value == nullptr)
        return def_value;

    char* end = nullptr;
    long num = strtol(value, &end, 10);
    if (*end != '\0')
        return def_value;

    return num;
}

void IniReader::GetSections(std::vector<std::string> &sections)
{
    sections.clear();
    for (sections_t::const_iterator iter = sections_.begin();
        iter != sections_.end(); ++iter)
    {
        sections.push_back(iter->first);
    }
}

bool IniReader::GetOptions(const char* section, std::vector<std::string> &options)
{
    return GetOptions(std::string(section), options);
}

bool IniReader::GetOptions(const std::string &section, std::vector<std::string> &options)
{
    sections_t::const_iterator iter = sections_.find(section);
    if (iter == sections_.end())
        return false;

    options.clear();
    for (options_t::const_iterator iter2 = iter->second.begin();
        iter2 != iter->second.end(); ++iter2)
    {
        options.push_back(iter2->first);
    }

    return true;
}

bool IniReader::string_caseless_comparer::operator() (const std::string &a, const char* b) const
{
    return strcasecmp(a.c_str(), b) < 0;
}

bool IniReader::string_caseless_comparer::operator() (const std::string &a, const std::string &b) const
{
    return strcasecmp(a.c_str(), b.c_str()) < 0;
}

}}  // namespace fasmio::common

