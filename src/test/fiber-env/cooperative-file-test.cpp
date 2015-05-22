
#include "gtest/gtest.h"

#include "runtime-env/fiber-env/cooperative-file.h"
#include "runtime-env/fiber-env/fiber-env.h"

using namespace std;
using namespace fasmio::fiber_env;

namespace {

#define MY_ASSERT(id, expr) context->numbers.push_back((expr) ? id : 999)

    static const char* const test_fn = "/tmp/cooperative-file-test";

    struct context_t {
        vector<int> numbers;
    };

    int fiber_write(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        CooperativeFile file;
        MY_ASSERT(1, file.Open(test_fn, "wb"));
        MY_ASSERT(2, 26 == file.Write("abcdefghijklmnopqrstuvwxyz", 26));
        MY_ASSERT(3, 10 == file.Write("0123456789", 10));
        MY_ASSERT(4, 26 == file.Write("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26));
        file.Close();

        return 0;
    }

    TEST(CooperativeFile, Write) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber_write, (void*)(&context), "fiber_write"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(4, context.numbers.size());
        for (int i=0; i<context.numbers.size(); ++i)
            EXPECT_EQ(i+1, context.numbers[i]);

        FILE *fp = fopen(test_fn, "rb");
        EXPECT_TRUE(nullptr != fp);
        char buff[128];
        memset(buff, 0, sizeof(buff));
        EXPECT_EQ(26+10+26, fread(buff, 1, 26+10+26, fp));
        EXPECT_EQ(0, strcmp(buff, "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
        fclose(fp);

        unlink(test_fn);
    }

    int fiber_read(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        CooperativeFile file;
        MY_ASSERT(1, file.Open(test_fn, "rb"));

        char buff[128];

        memset(buff, 0, sizeof(buff));
        MY_ASSERT(2, 26 == file.Read(buff, 26));
        MY_ASSERT(3, 0 == strcmp(buff, "abcdefghijklmnopqrstuvwxyz"));

        memset(buff, 0, sizeof(buff));
        MY_ASSERT(4, 10 == file.Read(buff, 10));
        MY_ASSERT(5, 0 == strcmp(buff, "0123456789"));

        memset(buff, 0, sizeof(buff));
        MY_ASSERT(6, 26 == file.Read(buff, 26));
        MY_ASSERT(7, 0 == strcmp(buff, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"));

        file.Close();
        return 0;
    }

    TEST(CooperativeFile, Read) {
        FiberEnv env;

        FILE *fp = fopen(test_fn, "wb");
        EXPECT_TRUE(nullptr != fp);
        EXPECT_EQ(26+10+26, fwrite("abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 1, 26+10+26, fp));
        fclose(fp);

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber_read, (void*)(&context), "fiber_read"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(7, context.numbers.size());
        for (int i=0; i<context.numbers.size(); ++i)
            EXPECT_EQ(i+1, context.numbers[i]);

        unlink(test_fn);
    }
}  // namespace <anonymous>

