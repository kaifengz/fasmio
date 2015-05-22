
#include "gtest/gtest.h"

#include "common/stream-adaptor.h"
#include "container/tlv-composer.h"

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

    TEST(TlvComposer, General)
    {
        std::ostringstream oss;
        fasmio::common::StdOStreamAdaptor adaptor(oss);

        fasmio::IOStream* vos = nullptr;

        TlvComposer composer(&adaptor);
        EXPECT_TRUE(composer.CheckIntegrity());
        EXPECT_TRUE(composer.BeginKey("root"));
            EXPECT_TRUE(!composer.CheckIntegrity());
            EXPECT_TRUE(composer.AddValue("name1", "value1"));
            EXPECT_TRUE(composer.AddValue("name2", reinterpret_cast<const unsigned char*>("value2"), 6));
            EXPECT_TRUE(composer.BeginKey("subkey"));
                EXPECT_TRUE(composer.AddValue("name", 123456L));
            EXPECT_TRUE(composer.EndKey("subkey"));
            EXPECT_TRUE(composer.BeginKey("subkey"));
                EXPECT_TRUE(composer.AddValue("name", 2345678901UL));
                EXPECT_TRUE(composer.BeginKey("subkey2"));
                    EXPECT_TRUE(composer.AddValue("name", 1234.5));
                EXPECT_TRUE(composer.EndKey("subkey2"));
            EXPECT_TRUE(composer.EndKey("subkey"));
            EXPECT_TRUE(nullptr != (vos = composer.BeginValue("name3")));
                EXPECT_TRUE(6 == vos->Write("value3", 6));
            EXPECT_TRUE(6 == composer.EndValue(vos));
            EXPECT_TRUE(nullptr != (vos = composer.BeginValue("name4")));
                EXPECT_TRUE(7 == vos->Write("several", 7));
                EXPECT_TRUE(5 == vos->Write("parts", 5));
            EXPECT_TRUE(12 == composer.EndValue(vos));
            EXPECT_TRUE(!composer.CheckIntegrity());
        EXPECT_TRUE(composer.EndKey("root"));
        EXPECT_TRUE(composer.CheckIntegrity());

        std::string expected(complex_data, sizeof(complex_data) - 1);
        EXPECT_EQ(expected, oss.str());
    }

}  // namespace <anonymous>

