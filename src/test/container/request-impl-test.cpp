
#include "gtest/gtest.h"

#include "container/request-impl.h"

using namespace fasmio::container;

namespace {

    TEST(RequestImpl, General)
    {
        RequestImpl req;
        EXPECT_TRUE(req.BuildFrom("http://host:8080/path?name=who%20cares&gender&age=34&age=45", "GET"));
        EXPECT_EQ(std::string("http://host:8080/path"), req.GetURL());
        EXPECT_EQ(std::string("GET"), req.GetMethod());
        EXPECT_EQ(std::string("who cares"), req.GetParameter("name"));
        EXPECT_EQ(std::string(""), req.GetParameter("gender"));
        EXPECT_EQ(std::string("45"), req.GetParameter("age"));
        EXPECT_EQ(nullptr, req.GetParameter("does-not-exist"));
        EXPECT_EQ(4, req.GetParameterCount());
        EXPECT_EQ(std::string("name"), req.GetParameterName(0));
        EXPECT_EQ(std::string("who cares"), req.GetParameterValue(0));
        EXPECT_EQ(std::string("gender"), req.GetParameterName(1));
        EXPECT_EQ(std::string(""), req.GetParameterValue(1));
        EXPECT_EQ(std::string("age"), req.GetParameterName(2));
        EXPECT_EQ(std::string("34"), req.GetParameterValue(2));
        EXPECT_EQ(std::string("age"), req.GetParameterName(3));
        EXPECT_EQ(std::string("45"), req.GetParameterValue(3));
        EXPECT_EQ(nullptr, req.GetParameterName(4));
        EXPECT_EQ(nullptr, req.GetParameterValue(4));
    }

}  // namespace <anonymous>

