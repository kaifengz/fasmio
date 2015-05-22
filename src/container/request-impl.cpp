
#include "./request-impl.h"
#include <assert.h>
#include <string.h>

namespace fasmio { namespace container {

RequestImpl::RequestImpl() :
    url_(),
    method_(),
    parameters_map_(),
    parameters_list_()
{
}

RequestImpl::~RequestImpl()
{
}

bool RequestImpl::BuildFrom(const char* full_url, const char* method)
{
    if (full_url == nullptr || method == nullptr)
        return false;

    method_ = method;
    parameters_map_.clear();
    parameters_list_.clear();

    const char* question_mark = strchr(full_url, '?');
    if (question_mark == nullptr)
        url_ = full_url;
    else
    {
        url_.assign(full_url, question_mark - full_url);

        const char* parameters = question_mark + 1;

        while (true)
        {
            const char* ampersand = strchr(parameters, '&');
            if (ampersand == nullptr)
                ampersand = parameters + strlen(parameters);

            const char* equals_sign = strchr(parameters, '=');
            if (equals_sign == nullptr || equals_sign > ampersand)
                equals_sign = ampersand;

            std::string name, value;
            name.assign(parameters, equals_sign - parameters);
            if (equals_sign != ampersand)
            {
                value.assign(equals_sign + 1, ampersand - equals_sign - 1);
                decode_url_characters(value);
            }

            parameters_map_[name] = value;
            parameters_list_.push_back(std::make_pair(name, value));

            if (*ampersand == '\0')
                break;
            parameters = ampersand + 1;
        }
    }

    return true;
}

bool RequestImpl::BuildFrom(INetRequest *req)
{
    if (req == nullptr)
        return false;

    return BuildFrom(req->GetURL(), req->GetMethod());
}

const char* RequestImpl::GetURL()
{
    return url_.c_str();
}

const char* RequestImpl::GetMethod()
{
    return method_.c_str();
}

const char* RequestImpl::GetParameter(const char *name)
{
    if (name == nullptr)
        return nullptr;

    parameters_map_t::const_iterator iter = parameters_map_.find(name);
    if (iter == parameters_map_.end())
        return nullptr;

    return iter->second.c_str();
}

unsigned int RequestImpl::GetParameterCount()
{
    return parameters_list_.size();
}

const char* RequestImpl::GetParameterName(unsigned int index)
{
    if (index >= parameters_list_.size())
        return nullptr;

    return parameters_list_[index].first.c_str();
}

const char* RequestImpl::GetParameterValue(unsigned int index)
{
    if (index >= parameters_list_.size())
        return nullptr;

    return parameters_list_[index].second.c_str();
}

void RequestImpl::decode_url_characters(std::string &s)
{
    if (s.find('%') == std::string::npos)
        return;

    std::string result;
    result.reserve(s.size());

    std::string::size_type pos = 0;
    while (true)
    {
        std::string::size_type percent_sign = s.find('%', pos);
        if (percent_sign == std::string::npos || percent_sign + 3 > s.size())
        {
            result.append(s, pos, std::string::npos);
            break;
        }

        char b1 = s[percent_sign+1], b2=s[percent_sign+2];
        if (!isxdigit(b1) || !isxdigit(b2))
            result.append(s, pos, percent_sign + 3 - pos);
        else
        {
            result.append(s, pos, percent_sign);

            int ch = 0;
            if (b1 >= '0' && b1 <= '9')
                ch += b1 - '0';
            else if (b1 >= 'a' && b1 <= 'f')
                ch += b1 - 'a' + 10;
            else // if (b1 >= 'A' && b1 <= 'F')
                ch += b1 - 'A' + 10;
            ch <<= 4;
            if (b2 >= '0' && b2 <= '9')
                ch += b2 - '0';
            else if (b2 >= 'a' && b2 <= 'f')
                ch += b2 - 'a' + 10;
            else // if (b2 >= 'A' && b2 <= 'F')
                ch += b2 - 'A' + 10;
            result.push_back((char)ch);
        }

        pos = percent_sign + 3;
    }

    s.swap(result);
}

}}  // namespace fasmio::container

