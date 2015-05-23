
#include "gtest/gtest.h"

#include <memory>
#include "container/task-meta-impl.h"
#include "common/stream-adaptor.h"

using namespace fasmio::container;
using namespace fasmio::service;

namespace {

    TEST(TaskMetaImpl, Serialize)
    {
        const char meta_str[] =
            "K\007TaskIds"
                "V\006TaskId\012my-task-id\0"
            "E"
            "K\011Generator"
                "V\004Node\007my-node\0"
                "V\007Service\012my-service\0"
                "V\011BirthTime\0131234.500000\0"
                "V\007NeedAck\0010\0"
                "V\013ResendCount\0012\0"
                "V\012ResendTime\0132345.000000\0"
            "E"
            // TODO: Journey
            // TODO: User
            ;

        TaskMetaImpl meta("my-task-id");

        meta.SetGeneratorInfo("my-node", "my-service", 1234.5, false);
        meta.SetResendInfo(2, 2345.0);

        std::ostringstream oss;
        fasmio::common::StdOStreamAdaptor adaptor(oss);
        meta.Serialize(&adaptor, TMF_TLV);

        EXPECT_EQ(std::string(meta_str, sizeof(meta_str)-1), oss.str());
    }
}

