
#include "gtest/gtest.h"

#include "common/ini-reader.h"
#include <sstream>

using namespace std;
using namespace fasmio::common;

namespace {

    TEST(IniReader, General)
    {
        istringstream is("\n\
            [first]\n\
            a = b\n\
            c = 123\n\
            \n\
            [empty-section]\n\
            \n\
            [second]\n\
            ; [third]\n\
            d = e\n\
            f = 456         \n\
            ");

        IniReader reader;
        EXPECT_TRUE(reader.Read(is));

        vector<string> names;

        reader.GetSections(names);
        EXPECT_EQ(3, names.size());

        EXPECT_TRUE(reader.GetOptions("first", names));
        EXPECT_EQ(2, names.size());

        EXPECT_TRUE(reader.GetOptions("empty-section", names));
        EXPECT_EQ(0, names.size());

        EXPECT_TRUE(reader.GetOptions("second", names));
        EXPECT_EQ(2, names.size());

        EXPECT_STREQ("b", reader.GetStrValue("FIRST", "a", nullptr));
        EXPECT_EQ(123, reader.GetIntValue("first", "c", 0));
        EXPECT_STREQ("e", reader.GetStrValue("second", "d", nullptr));
        EXPECT_EQ(456, reader.GetIntValue("SECOND", "f", 0));
    }

}  // namespace <anonymous>

