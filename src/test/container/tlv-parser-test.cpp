
#include "gtest/gtest.h"

#include "common/stream-adaptor.h"
#include "container/tlv-parser.h"

using namespace fasmio::container;

namespace {

    static const char complex_data[] =
        "K\004root"
            "V\005name1\006value1\000"
            "V\005name2\006value2\000"
            "K\006subkey"
                "V\004name\006123456\000"
            "E"
            "K\006subkey"
                "V\004name\0122345678901\000"
                "K\007subkey2"
                    "V\004name\0131234.500000\000"
                "E"
            "E"
            "V\005name3\006value3\000"
            "V\005name4\007several\005parts\000"
        "E";

    TEST(TlvParser, General)
    {
#define TEST_STR(str, result) \
        do { \
            std::istringstream iss(std::string(str, sizeof(str)-1)); \
            fasmio::common::StdIStreamAdaptor adaptor(iss); \
            TlvParser parser(&adaptor); \
            EXPECT_TRUE(result == parser.Parse()); \
        } while (0)
#define GOOD(str) TEST_STR(str, true)
#define BAD(str)  TEST_STR(str, false)

        BAD("");
        BAD("k");
        BAD("v");
        BAD("e");

        GOOD("k\004roote");
        GOOD("v\004name\005value\000");

        BAD("k\004root");  // missing the ending 'e'
        BAD("v\004name\005va");  // unexpected value end

        GOOD(complex_data);

#undef TEST_STR
#undef GOOD
#undef BAD
    }

    struct context_t
    {
        std::vector<std::string> current_key;
        std::vector<std::string> names;

        std::string get_current_key() const
        {
            if (current_key.empty())
                return "/";

            std::string key;
            for (int i=0; i<current_key.size(); ++i)
                key += "/" + current_key[i];
            return key;
        }
    };

    bool enter_key(const char* name, void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->current_key.push_back(name);
        context->names.push_back(context->get_current_key());
        return true;
    }

    bool leave_key(const char* name, void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->current_key.pop_back();
        return true;
    }

    TEST(TlvParser, Keys)
    {
        std::istringstream iss(std::string(complex_data, sizeof(complex_data)-1));
        fasmio::common::StdIStreamAdaptor adaptor(iss);

        context_t context;
        TlvParser parser(&adaptor);
        parser.SetEnterKeyCallback(enter_key, &context);
        parser.SetLeaveKeyCallback(leave_key, &context);
        EXPECT_TRUE(parser.Parse());

        EXPECT_EQ(4, context.names.size());
        EXPECT_EQ("/root", context.names[0]);
        EXPECT_EQ("/root/subkey", context.names[1]);
        EXPECT_EQ("/root/subkey", context.names[2]);
        EXPECT_EQ("/root/subkey/subkey2", context.names[3]);
    }

    bool enter_key2(const char* name, void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->current_key.push_back(name);
        return true;
    }

    bool leave_key2(const char* name, void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->current_key.pop_back();
        return true;
    }

    bool value_istream(const char* name, fasmio::IIStream *is, void *arg)
    {
        std::string value;
        while (true)
        {
            char buff[512];
            long bytes = is->Read(buff, sizeof(buff));
            if (bytes <= 0)
                break;
            value.append(buff, bytes);
        }

        context_t *context = reinterpret_cast<context_t*>(arg);
        context->names.push_back(context->get_current_key() + "/" + name + "=" + value);
        return true;
    }

    TEST(TlvParser, ValueIStream)
    {
        std::istringstream iss(std::string(complex_data, sizeof(complex_data)-1));
        fasmio::common::StdIStreamAdaptor adaptor(iss);

        context_t context;
        TlvParser parser(&adaptor);
        parser.SetEnterKeyCallback(enter_key2, &context);
        parser.SetLeaveKeyCallback(leave_key2, &context);
        parser.SetValueCallback(value_istream, &context);
        EXPECT_TRUE(parser.Parse());

        EXPECT_EQ(7, context.names.size());
        EXPECT_EQ("/root/name1=value1", context.names[0]);
        EXPECT_EQ("/root/name2=value2", context.names[1]);
        EXPECT_EQ("/root/subkey/name=123456", context.names[2]);
        EXPECT_EQ("/root/subkey/name=2345678901", context.names[3]);
        EXPECT_EQ("/root/subkey/subkey2/name=1234.500000", context.names[4]);
        EXPECT_EQ("/root/name3=value3", context.names[5]);
        EXPECT_EQ("/root/name4=severalparts", context.names[6]);
    }

    bool value_string(const char* name, const char* value, unsigned int value_len, void *arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->names.push_back(context->get_current_key() + "/" + name + "=" + std::string(value, value_len));
        return true;
    }

    TEST(TlvParser, ValueString)
    {
        std::istringstream iss(std::string(complex_data, sizeof(complex_data)-1));
        fasmio::common::StdIStreamAdaptor adaptor(iss);

        context_t context;
        TlvParser parser(&adaptor);
        parser.SetEnterKeyCallback(enter_key2, &context);
        parser.SetLeaveKeyCallback(leave_key2, &context);
        parser.SetValueCallback(value_string, &context);
        EXPECT_TRUE(parser.Parse());

        EXPECT_EQ(7, context.names.size());
        EXPECT_EQ("/root/name1=value1", context.names[0]);
        EXPECT_EQ("/root/name2=value2", context.names[1]);
        EXPECT_EQ("/root/subkey/name=123456", context.names[2]);
        EXPECT_EQ("/root/subkey/name=2345678901", context.names[3]);
        EXPECT_EQ("/root/subkey/subkey2/name=1234.500000", context.names[4]);
        EXPECT_EQ("/root/name3=value3", context.names[5]);
        EXPECT_EQ("/root/name4=severalparts", context.names[6]);
    }

}  // namespace <anonymous>

