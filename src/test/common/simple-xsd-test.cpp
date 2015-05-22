
#include "gtest/gtest.h"

#include "common/simple-xsd.h"
#include "common/stream-adaptor.h"
#include <sstream>

using namespace std;
using namespace fasmio::common;
using namespace fasmio::common::simple_xsd;

namespace {

    class a_structure
    {
    public:
        bool serialize(ostream &os)
        {
            StdOStreamAdaptor adaptor(os);
            composer composer(&adaptor);
            return composer.compose(*this, "a_structure");
        }

        bool unserialize(istream &is)
        {
            StdIStreamAdaptor adaptor(is);
            parser parser(&adaptor);
            return parser.parse(*this, "a_structure");
        }

    public:
        class b_inner_structure
        {
        public:
            b_inner_structure()
            {
            }

            b_inner_structure(int i, string s) :
                b_int(i), b_string(s)
            {
            }

        public:
            int b_int;
            string b_string;
        };

    public:
        char a_char;
        unsigned char a_uchar;
        short a_short;
        unsigned short a_ushort;
        int a_int;
        unsigned int a_uint;
        long a_long;
        unsigned long a_ulong;
        long long a_long_long;
        unsigned long long a_ulong_long;
        float a_float;
        double a_double;
        string a_string;
        std::unique_ptr<string> a_string_opt;
        std::vector<string> a_string_sequence;
        b_inner_structure a_bis;
        std::unique_ptr<b_inner_structure> a_bis_opt;
        std::vector<b_inner_structure> a_bis_sequence;
    };

}  // namespace <anonymous>

namespace fasmio { namespace common { namespace simple_xsd {

    template <>
    struct field_parser<a_structure>
    {
        static bool parse_field(parser &parser, a_structure &data)
        {
            return
                parser.parse_field(data.a_char,             "a_char"            ) &&
                parser.parse_field(data.a_uchar,            "a_uchar"           ) &&
                parser.parse_field(data.a_short,            "a_short"           ) &&
                parser.parse_field(data.a_ushort,           "a_ushort"          ) &&
                parser.parse_field(data.a_int,              "a_int"             ) &&
                parser.parse_field(data.a_uint,             "a_uint"            ) &&
                parser.parse_field(data.a_long,             "a_long"            ) &&
                parser.parse_field(data.a_ulong,            "a_ulong"           ) &&
                parser.parse_field(data.a_long_long,        "a_long_long"       ) &&
                parser.parse_field(data.a_ulong_long,       "a_ulong_long"      ) &&
                parser.parse_field(data.a_float,            "a_float"           ) &&
                parser.parse_field(data.a_double,           "a_double"          ) &&
                parser.parse_field(data.a_string,           "a_string"          ) &&
                parser.parse_field(data.a_string_opt,       "a_string_opt"      ) &&
                parser.parse_field(data.a_string_sequence,  "a_string_sequence" ) &&
                parser.parse_field(data.a_bis,              "a_bis"             ) &&
                parser.parse_field(data.a_bis_opt,          "a_bis_opt"         ) &&
                parser.parse_field(data.a_bis_sequence,     "a_bis_sequence"    );
        }
    };

    template <>
    struct field_composer<a_structure>
    {
        static bool compose_field(composer &composer, const a_structure &data)
        {
            return
                composer.compose_field(data.a_char,             "a_char"            ) &&
                composer.compose_field(data.a_uchar,            "a_uchar"           ) &&
                composer.compose_field(data.a_short,            "a_short"           ) &&
                composer.compose_field(data.a_ushort,           "a_ushort"          ) &&
                composer.compose_field(data.a_int,              "a_int"             ) &&
                composer.compose_field(data.a_uint,             "a_uint"            ) &&
                composer.compose_field(data.a_long,             "a_long"            ) &&
                composer.compose_field(data.a_ulong,            "a_ulong"           ) &&
                composer.compose_field(data.a_long_long,        "a_long_long"       ) &&
                composer.compose_field(data.a_ulong_long,       "a_ulong_long"      ) &&
                composer.compose_field(data.a_float,            "a_float"           ) &&
                composer.compose_field(data.a_double,           "a_double"          ) &&
                composer.compose_field(data.a_string,           "a_string"          ) &&
                composer.compose_field(data.a_string_opt,       "a_string_opt"      ) &&
                composer.compose_field(data.a_string_sequence,  "a_string_sequence" ) &&
                composer.compose_field(data.a_bis,              "a_bis"             ) &&
                composer.compose_field(data.a_bis_opt,          "a_bis_opt"         ) &&
                composer.compose_field(data.a_bis_sequence,     "a_bis_sequence"    );
        }
    };

    template <>
    struct field_parser<a_structure::b_inner_structure>
    {
        static bool parse_field(parser &parser, a_structure::b_inner_structure &data)
        {
            return
                parser.parse_field(data.b_int,     "b_int"    ) &&
                parser.parse_field(data.b_string,  "b_string" );
        }
    };

    template <>
    struct field_composer<a_structure::b_inner_structure>
    {
        static bool compose_field(composer &composer, const a_structure::b_inner_structure &data)
        {
            return
                composer.compose_field(data.b_int,     "b_int"    ) &&
                composer.compose_field(data.b_string,  "b_string" );
        }
    };


}}}  // namespace fasmio::common::simple_xsd


namespace {

    TEST(SimpleXSD, General) {
        ostringstream oss;
        stringstream ss;

        {   a_structure data;
            data.a_char = 'C';
            data.a_uchar = 0xCC;
            data.a_short = -123;
            data.a_ushort = 65500;
            data.a_int = -456;
            data.a_uint = 4294000000U;
            data.a_long = 1234567890L;
            data.a_ulong = 3456789012UL;
            data.a_long_long = 1234567890123456789LL;
            data.a_ulong_long = 12345678901234567890ULL;
            data.a_float = 1.5;
            data.a_double = 3.5;
            data.a_string = "a short string";
            data.a_string_sequence.push_back("seq-1");
            data.a_string_sequence.push_back("seq-2");
            data.a_bis.b_int = 9;
            data.a_bis.b_string = "bis";
            data.a_bis_opt.reset(new a_structure::b_inner_structure(19, "bis-opt"));
            data.a_bis_sequence.push_back(a_structure::b_inner_structure(29, "first"));
            data.a_bis_sequence.push_back(a_structure::b_inner_structure(39, "second"));
            data.a_bis_sequence.push_back(a_structure::b_inner_structure(49, "third"));

            data.serialize(ss);
            data.serialize(oss);
        }

        EXPECT_EQ(
            "<a_structure>"
                "<a_char>67</a_char>"
                "<a_uchar>204</a_uchar>"
                "<a_short>-123</a_short>"
                "<a_ushort>65500</a_ushort>"
                "<a_int>-456</a_int>"
                "<a_uint>4294000000</a_uint>"
                "<a_long>1234567890</a_long>"
                "<a_ulong>3456789012</a_ulong>"
                "<a_long_long>1234567890123456789</a_long_long>"
                "<a_ulong_long>12345678901234567890</a_ulong_long>"
                "<a_float>1.500000</a_float>"
                "<a_double>3.500000</a_double>"
                "<a_string>a short string</a_string>"
                "<a_string_sequence>seq-1</a_string_sequence>"
                "<a_string_sequence>seq-2</a_string_sequence>"
                "<a_bis>"
                    "<b_int>9</b_int>"
                    "<b_string>bis</b_string>"
                "</a_bis>"
                "<a_bis_opt>"
                    "<b_int>19</b_int>"
                    "<b_string>bis-opt</b_string>"
                "</a_bis_opt>"
                "<a_bis_sequence>"
                    "<b_int>29</b_int>"
                    "<b_string>first</b_string>"
                "</a_bis_sequence>"
                "<a_bis_sequence>"
                    "<b_int>39</b_int>"
                    "<b_string>second</b_string>"
                "</a_bis_sequence>"
                "<a_bis_sequence>"
                    "<b_int>49</b_int>"
                    "<b_string>third</b_string>"
                "</a_bis_sequence>"
            "</a_structure>\n",
            oss.str());

        {   a_structure data2;
            data2.unserialize(ss);

            EXPECT_EQ('C', data2.a_char);
            EXPECT_EQ(0xCC, data2.a_uchar);
            EXPECT_EQ(-123, data2.a_short);
            EXPECT_EQ(65500, data2.a_ushort);
            EXPECT_EQ(-456, data2.a_int);
            EXPECT_EQ(4294000000U, data2.a_uint);
            EXPECT_EQ(1234567890L, data2.a_long);
            EXPECT_EQ(3456789012UL, data2.a_ulong);
            EXPECT_EQ(1234567890123456789LL, data2.a_long_long);
            EXPECT_EQ(12345678901234567890ULL, data2.a_ulong_long);
            EXPECT_DOUBLE_EQ(1.5, data2.a_float);
            EXPECT_DOUBLE_EQ(3.5, data2.a_double);
            EXPECT_EQ("a short string", data2.a_string);
            EXPECT_TRUE(data2.a_string_opt.get() == nullptr);
            EXPECT_EQ(2, data2.a_string_sequence.size());
            EXPECT_EQ("seq-1", data2.a_string_sequence[0]);
            EXPECT_EQ("seq-2", data2.a_string_sequence[1]);
            EXPECT_EQ(9, data2.a_bis.b_int);
            EXPECT_EQ("bis", data2.a_bis.b_string);
            EXPECT_TRUE(data2.a_bis_opt.get() != nullptr);
            EXPECT_EQ(19, data2.a_bis_opt->b_int);
            EXPECT_EQ("bis-opt", data2.a_bis_opt->b_string);
            EXPECT_EQ(3, data2.a_bis_sequence.size());
            EXPECT_EQ(29, data2.a_bis_sequence[0].b_int);
            EXPECT_EQ("first", data2.a_bis_sequence[0].b_string);
            EXPECT_EQ(39, data2.a_bis_sequence[1].b_int);
            EXPECT_EQ("second", data2.a_bis_sequence[1].b_string);
            EXPECT_EQ(49, data2.a_bis_sequence[2].b_int);
            EXPECT_EQ("third", data2.a_bis_sequence[2].b_string);
        }
    }

}  // namespace <anonymous>

