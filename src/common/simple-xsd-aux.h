
#ifndef COMMON_SIMPLE_XSD_H_
#   error Do not include "simple-xsd-aux.h" directly, include "simple-xsd.h" instead
#endif

#ifndef COMMON_SIMPLE_XSD_AUX_H_
#define COMMON_SIMPLE_XSD_AUX_H_

#include <stdio.h>
#include <string.h>

namespace fasmio { namespace common { namespace simple_xsd {

    /////////////////////////////////////////////////////////////////////////
    // field_validator && field_trait

    struct field_validator__composed_data_type
    {
        static bool validate(rapidxml::xml_node<> *node)
        {
            rapidxml::xml_node<> *first_child = node->first_node();
            if (first_child == nullptr)
                return false;

            return (first_child->type() != rapidxml::node_data);
        }
    };

    struct field_validator__native_data_type
    {
        static bool validate(rapidxml::xml_node<> *node)
        {
            rapidxml::xml_node<> *first_child = node->first_node();
            if (first_child == nullptr)
                return false;

            return (first_child->type() == rapidxml::node_data);
        }
    };

    template <typename T>
    struct field_trait
    {
        typedef field_validator__composed_data_type validator;
    };

    /////////////////////////////////////////////////////////////////////////
    // base-type field_trait

#define DECLARE_BASE_TYPE_TRAIT(Type) \
    template <> \
    struct field_trait<Type> { \
        typedef field_validator__native_data_type validator; \
    };

    DECLARE_BASE_TYPE_TRAIT(long);
    DECLARE_BASE_TYPE_TRAIT(unsigned long);
    DECLARE_BASE_TYPE_TRAIT(long long);
    DECLARE_BASE_TYPE_TRAIT(unsigned long long);
    DECLARE_BASE_TYPE_TRAIT(double);
    DECLARE_BASE_TYPE_TRAIT(std::string);

#undef DECLARE_BASE_TYPE_TRAIT

    /////////////////////////////////////////////////////////////////////////
    // base-type field_parser

    template <>
    struct field_parser<long>
    {
        static bool parse_field(parser &parser, long &n)
        {
            const char* str = parser.get_string_and_advance();
            if (str == nullptr)
                return false;

            char* end = nullptr;
            long num = strtol(str, &end, 10);
            if (*end != '\0')
                return false;

            n = num;
            return true;
        }
    };

    template <>
    struct field_parser<unsigned long>
    {
        static bool parse_field(parser &parser, unsigned long &n)
        {
            const char* str = parser.get_string_and_advance();
            if (str == nullptr)
                return false;

            char* end = nullptr;
            long num = strtoul(str, &end, 10);
            if (*end != '\0')
                return false;

            n = num;
            return true;
        }
    };

    template <>
    struct field_parser<long long>
    {
        static bool parse_field(parser &parser, long long &n)
        {
            const char* str = parser.get_string_and_advance();
            if (str == nullptr)
                return false;

            char* end = nullptr;
            long long num = strtoll(str, &end, 10);
            if (*end != '\0')
                return false;

            n = num;
            return true;
        }
    };

    template <>
    struct field_parser<unsigned long long>
    {
        static bool parse_field(parser &parser, unsigned long long &n)
        {
            const char* str = parser.get_string_and_advance();
            if (str == nullptr)
                return false;

            char* end = nullptr;
            long long num = strtoull(str, &end, 10);
            if (*end != '\0')
                return false;

            n = num;
            return true;
        }
    };

    template <>
    struct field_parser<double>
    {
        static bool parse_field(parser &parser, double &n)
        {
            const char* str = parser.get_string_and_advance();
            if (str == nullptr)
                return false;

            char* end = nullptr;
            double num = strtod(str, &end);
            if (*end != '\0')
                return false;

            n = num;
            return true;
        }
    };

    template <>
    struct field_parser<std::string>
    {
        static bool parse_field(parser &parser, std::string &s)
        {
            const char* str = parser.get_string_and_advance();
            if (str == nullptr)
                return false;

            s = str;
            return true;
        }
    };

    /////////////////////////////////////////////////////////////////////////
    // base-type field_composer

    template <>
    struct field_composer<long>
    {
        static bool compose_field(composer &composer, const long &n)
        {
            char buff[128];
            int length = snprintf(buff, sizeof(buff), "%ld", n);
            return composer.put_string(buff, length);
        }
    };

    template <>
    struct field_composer<unsigned long>
    {
        static bool compose_field(composer &composer, const unsigned long &n)
        {
            char buff[128];
            int length = snprintf(buff, sizeof(buff), "%lu", n);
            return composer.put_string(buff, length);
        }
    };

    template <>
    struct field_composer<long long>
    {
        static bool compose_field(composer &composer, const long long &n)
        {
            char buff[128];
            int length = snprintf(buff, sizeof(buff), "%lld", n);
            return composer.put_string(buff, length);
        }
    };

    template <>
    struct field_composer<unsigned long long>
    {
        static bool compose_field(composer &composer, const unsigned long long &n)
        {
            char buff[128];
            int length = snprintf(buff, sizeof(buff), "%llu", n);
            return composer.put_string(buff, length);
        }
    };

    template <>
    struct field_composer<double>
    {
        static bool compose_field(composer &composer, const double &n)
        {
            char buff[128];
            int length = snprintf(buff, sizeof(buff), "%f", n);
            return composer.put_string(buff, length);
        }
    };

    template <>
    struct field_composer<std::string>
    {
        static bool compose_field(composer &composer, const std::string &str)
        {
            // TODO: escape XML characters
            return composer.put_string(str.c_str(), str.size());
        }
    };

    /////////////////////////////////////////////////////////////////////////
    // dependant-type field_trait, field_parser and field_composer

#define DECLARE_DEPENDANT_TYPE_TRAIT(Type, BaseType) \
    template <> \
    struct field_trait<Type> { \
        typedef field_validator__native_data_type validator; \
    }; \
    template <> \
    struct field_parser<Type> { \
        static bool parse_field(parser &parser, Type &data) { \
            BaseType base_data; \
            if (!field_parser<BaseType>::parse_field(parser, base_data)) \
                return false; \
            data = base_data; \
            return true; \
        } \
    }; \
    template <> \
    struct field_composer<Type> { \
        static bool compose_field(composer &composer, const Type &data) { \
            return field_composer<BaseType>::compose_field(composer, static_cast<const BaseType &>(data)); \
        } \
    }

    DECLARE_DEPENDANT_TYPE_TRAIT(char,               long);
    DECLARE_DEPENDANT_TYPE_TRAIT(short,              long);
    DECLARE_DEPENDANT_TYPE_TRAIT(int,                long);
    DECLARE_DEPENDANT_TYPE_TRAIT(unsigned char,      unsigned long);
    DECLARE_DEPENDANT_TYPE_TRAIT(unsigned short,     unsigned long);
    DECLARE_DEPENDANT_TYPE_TRAIT(unsigned int,       unsigned long);
    DECLARE_DEPENDANT_TYPE_TRAIT(float,              double);

#undef DECLARE_DEPENDANT_TYPE_TRAIT

    /////////////////////////////////////////////////////////////////////////
    // parser

    template <int unused>
    basic_parser<unused>::basic_parser(IIStream *is) :
        is_(is)
    {
    }

    template <int unused>
    template <typename T>
    bool basic_parser<unused>::parse(T &t, const char* tag /* = nullptr */)
    {
        if (is_ == nullptr)
            return false;

        std::string str;
        while (true)
        {
            char buff[1024*4];
            long n = is_->Read(buff, sizeof(buff));
            if (n <= 0)
                break;
            str.append(buff, n);
        }

        rapidxml::xml_document<> doc;
        try
        {
            doc.parse<0>(const_cast<char*>(str.c_str()));
        }
        catch (rapidxml::parse_error &err)
        {
            return false;
        }

        node_ = doc.first_node();
        if (node_ == nullptr)
            return false;

        return parse_field(t, tag != nullptr ? tag : node_->name());
    }

    template <int unused>
    template <typename T>
    bool basic_parser<unused>::parse_field(T &t, const char* tag)
    {
        if (node_ == nullptr)
            return false;
        if (0 != strcmp(node_->name(), tag))
            return false;
        if (!field_trait<T>::validator::validate(node_))
            return false;

        rapidxml::xml_node<> *next = node_->next_sibling();
        node_ = node_->first_node();
        bool result = field_parser<T>::parse_field(*this, t);
        node_ = next;
        return result;
    }

    template <int unused>
    template <typename T, typename Alloc>
    bool basic_parser<unused>::parse_field(std::vector<T, Alloc> &fields, const char* tag)
    {
        std::vector<T, Alloc> parsed_fields;

        while (true)
        {
            if (node_ == nullptr)
                break;
            if (0 != strcmp(node_->name(), tag))
                break;

            parsed_fields.resize( parsed_fields.size() + 1 );

            rapidxml::xml_node<> *next = node_->next_sibling();
            node_ = node_->first_node();
            bool result = field_parser<T>::parse_field(*this, parsed_fields.back());
            node_ = next;
            if (!result)
                return false;
        }

        fields.swap(parsed_fields);
        return true;
    }

    template <int unused>
    template <typename T>
    bool basic_parser<unused>::parse_field(std::unique_ptr<T> &opt, const char* tag)
    {
        if (node_ != nullptr &&
            0 == strcmp(node_->name(), tag) &&
            field_trait<T>::validator::validate(node_))
        {
            if (opt.get() == nullptr)
                opt.reset(new T);

            rapidxml::xml_node<> *next = node_->next_sibling();
            node_ = node_->first_node();
            bool result = field_parser<T>::parse_field(*this, *opt);
            node_ = next;
            return result;
        }
        else
        {
            opt.reset();
            return true;
        }
    }

    template <int unused>
    const char* basic_parser<unused>::get_string_and_advance()
    {
        if (node_ == nullptr)
            return nullptr;

        const char *result = node_->value();
        node_ = node_->next_sibling();
        return result;
    }

    /////////////////////////////////////////////////////////////////////////
    // composer

    template <int unused>
    composer_template<unused>::composer_template(IOStream *os) :
        os_(os)
    {
    }

    template <int unused>
    template <typename T>
    bool composer_template<unused>::compose(const T &t, const char* tag)
    {
        return compose_field(t, tag) && put_string("\n", 1);
    }

    template <int unused>
    template <typename T>
    bool composer_template<unused>::compose_field(const T &t, const char* tag, int length /* = -1 */)
    {
        if (length < 0)
            length = strlen(tag);

        return
            open_element(tag, length) &&
            field_composer<T>::compose_field(*this, t) &&
            close_element(tag, length);
    }

    template <int unused>
    template <typename T, typename Alloc>
    bool composer_template<unused>::compose_field(const std::vector<T, Alloc> &fields, const char* tag, int length /* = -1 */)
    {
        if (length < 0)
            length = strlen(tag);

        for (typename std::vector<T, Alloc>::const_iterator iter = fields.begin();
            iter != fields.end(); ++iter)
        {
            if (!compose_field(*iter, tag, length))
                return false;
        }
        return true;
    }

    template <int unused>
    template <typename T>
    bool composer_template<unused>::compose_field(const std::unique_ptr<T> &opt, const char* tag, int length /* = -1 */)
    {
        if (opt.get() == nullptr)
            return true;

        return compose_field(*opt, tag, length);
    }

    template <int unused>
    bool composer_template<unused>::open_element(const char* tag, int length)
    {
        if (length < 0)
            length = strlen(tag);

        return put_string("<", 1) && put_string(tag, length) && put_string(">", 1);
    }

    template <int unused>
    bool composer_template<unused>::close_element(const char* tag, int length)
    {
        if (length < 0)
            length = strlen(tag);

        return put_string("</", 2) && put_string(tag, length) && put_string(">", 1);
    }

    template <int unused>
    bool composer_template<unused>::put_string(const char* str)
    {
        int length = strlen(str);
        return length == os_->Write(str, length);
    }

    template <int unused>
    bool composer_template<unused>::put_string(const char* str, int length)
    {
        return length == os_->Write(str, length);
    }

}}} // namespace fasmio::common::simple_xsd

#endif  // COMMON_SIMPLE_XSD_AUX_H_

