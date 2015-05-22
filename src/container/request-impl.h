
#ifndef CONTAINER_REQUEST_IMPL_H_
#define CONTAINER_REQUEST_IMPL_H_

#include "interface/net-request.h"
#include "service/interface/request.h"
#include <map>
#include <string>
#include <vector>

namespace fasmio { namespace container {

class RequestImpl : public service::IRequest
{
public:
    RequestImpl();
    virtual ~RequestImpl();

    bool BuildFrom(const char* full_url, const char* method);
    bool BuildFrom(INetRequest*);

public:
    virtual const char*  GetURL();
    virtual const char*  GetMethod();
    virtual const char*  GetParameter(const char*);
    virtual unsigned int GetParameterCount();
    virtual const char*  GetParameterName(unsigned int index);
    virtual const char*  GetParameterValue(unsigned int index);

private:
    void decode_url_characters(std::string &);

private:
    typedef std::map<std::string, std::string> parameters_map_t;
    typedef std::vector<std::pair<std::string, std::string> > parameters_list_t;

    std::string url_;
    std::string method_;
    parameters_map_t parameters_map_;
    parameters_list_t parameters_list_;
};

}}  // namespace fasmio::container

#endif  // CONTAINER_REQUEST_IMPL_H_

