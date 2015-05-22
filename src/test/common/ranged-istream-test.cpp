
#include "gtest/gtest.h"

#include "common/ranged-istream.h"
#include "common/stream-adaptor.h"

using namespace fasmio::common;

namespace {

    TEST(RangedStdIStream, General)
    {
        char buff[256];

        std::istringstream iss("abcdefghijklmnopqrstuvwxyz");

        iss.read(buff, 7);
        EXPECT_EQ(7, iss.gcount());
        EXPECT_EQ(std::string("abcdefg"), std::string(buff, 7));

        RangedStdIStream ris(iss, 7);
        ris.read(buff, sizeof(buff));
        EXPECT_EQ(7, ris.gcount());
        EXPECT_EQ(std::string("hijklmn"), std::string(buff, 7));

        iss.read(buff, sizeof(buff));
        EXPECT_EQ(12, iss.gcount());
        EXPECT_EQ(std::string("opqrstuvwxyz"), std::string(buff, 12));
    }

    TEST(RangedIStream, General)
    {
        char buff[256];

        std::istringstream iss("abcdefghijklmnopqrstuvwxyz");
        StdIStreamAdaptor adaptor(iss);

        EXPECT_EQ(7, adaptor.Read(buff, 7));
        EXPECT_EQ(std::string("abcdefg"), std::string(buff, 7));

        RangedIStream ris(&adaptor, 7);
        EXPECT_EQ(7, ris.Read(buff, sizeof(buff)));
        EXPECT_EQ(std::string("hijklmn"), std::string(buff, 7));

        EXPECT_EQ(12, adaptor.Read(buff, sizeof(buff)));
        EXPECT_EQ(std::string("opqrstuvwxyz"), std::string(buff, 12));
    }

}  // namespace <anonymous>

