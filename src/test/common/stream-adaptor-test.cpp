
#include "gtest/gtest.h"

#include "common/stream-adaptor.h"
#include <sstream>

using namespace std;
using namespace fasmio::common;

namespace {

    class SimpleOStream : public fasmio::IOStream
    {
        ostringstream os_;

    public:
        virtual long Write(const void* buff, long size)
        {
            os_.write(reinterpret_cast<const char*>(buff), size);
            return size;
        }

        string str()
        {
            return os_.str();
        }
    };

    TEST(OStreamAdaptor, General)
    {
        SimpleOStream os;
        OStreamAdaptor adaptor(&os);

        adaptor << 123 << "abc" << '-' << endl;

        string s("123abc-\n");
        EXPECT_EQ(s, os.str());
    }

    TEST(OStreamAdaptor, Overflow)
    {
        SimpleOStream os;
        OStreamAdaptor adaptor(&os);

        for (int i=0; i<1000; ++i)
            adaptor.write("abcdefghijklmnopqrstuvwxyz", 26);
        adaptor.flush();

        string s(os.str());
        EXPECT_EQ(1000 * 26, s.size());
        for (int i=0; i<1000; ++i)
            EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", s.substr(i*26, 26));
    }

    class SimpleIStream : public fasmio::IIStream
    {
        istringstream is_;

    public:
        explicit SimpleIStream(const char* init) :
            is_(init)
        {
        }

        virtual long Read(void* buff, long size)
        {
            if (is_.eof())
                return -1;

            is_.read(reinterpret_cast<char*>(buff), size);
            return is_.gcount();
        }
    };

    TEST(IStreamAdaptor, General)
    {
        SimpleIStream is("123\nabc\n-");
        IStreamAdaptor adaptor(&is);

        int n;
        string s;
        char c;
        adaptor >> n >> s >> c;
        EXPECT_EQ(123, n);
        EXPECT_EQ("abc", s);
        EXPECT_EQ('-', c);
    }

    TEST(IStreamAdaptor, Underflow)
    {
        string s("abcdefghijklmnopqrstuvwxyz");
        for (int i=0; i<8; ++i)
            s += s;
        EXPECT_EQ(26 * 256, s.size());

        SimpleIStream is(s.c_str());
        IStreamAdaptor adaptor(&is);

        for (int i=0; i<256; ++i)
        {
            char buff[32];
            adaptor.read(buff, 26);
            EXPECT_EQ(26, adaptor.gcount());
            buff[26] = '\0';
            EXPECT_EQ("abcdefghijklmnopqrstuvwxyz", std::string(buff));
        }

        EXPECT_FALSE(adaptor.eof());

        char ch;
        adaptor >> ch;
        EXPECT_TRUE(adaptor.eof());
    }

}  // namespace <anonymous>

