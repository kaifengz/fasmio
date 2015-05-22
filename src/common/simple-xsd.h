
#ifndef COMMON_SIMPLE_XSD_H_
#define COMMON_SIMPLE_XSD_H_

#include <istream>
#include <ostream>
#include <memory>
#include <string>
#include <vector>

#include "rapidxml/rapidxml.hpp"
#include "interface/stream.h"

namespace fasmio { namespace common { namespace simple_xsd {

    /////////////////////////////////////////////////////////////////////////
    // parser

    template <int unused>
    class basic_parser
    {
    public:
        explicit basic_parser(IIStream *);

    public:
        template <typename T>
        bool parse(T &, const char* tag=nullptr);

    public:
        template <typename T>
        bool parse_field(T &, const char* tag);

        template <typename T, typename Alloc>
        bool parse_field(std::vector<T, Alloc> &, const char* tag);

        template <typename T>
        bool parse_field(std::unique_ptr<T> &, const char* tag);

    public:
        const char* get_string_and_advance();

    private:
        IIStream *is_;
        rapidxml::xml_node<> *node_;
    };

    typedef basic_parser<0> parser;

    template <typename T>
    struct field_parser
    {
        static bool parse_field(parser &, T &);
    };


    /////////////////////////////////////////////////////////////////////////
    // composer

    template <int unused>
    class composer_template
    {
    public:
        explicit composer_template(IOStream *is);

    public:
        template <typename T>
        bool compose(const T &, const char* tag);

    public:
        template <typename T>
        bool compose_field(const T &, const char* tag, int length = -1);

        template <typename T, typename Alloc>
        bool compose_field(const std::vector<T, Alloc> &, const char* tag, int length = -1);

        template <typename T>
        bool compose_field(const std::unique_ptr<T> &, const char* tag, int length = -1);

    public:
        bool open_element(const char* tag, int length = -1);
        bool close_element(const char* tag, int length = -1);

        bool put_string(const char*);
        bool put_string(const char*, int length);

    private:
        IOStream *os_;
    };

    typedef composer_template<0> composer;

    template <typename T>
    struct field_composer
    {
        static bool compose_field(composer &, const T &);
    };

}}} // namespace fasmio::common::simple_xsd

#include "simple-xsd-aux.h"

#endif  // COMMON_SIMPLE_XSD_H_

