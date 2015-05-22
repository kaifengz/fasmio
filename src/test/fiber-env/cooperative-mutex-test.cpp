
#include "gtest/gtest.h"

#include "runtime-env/fiber-env/cooperative-mutex.h"
#include "runtime-env/fiber-env/fiber-env.h"

using namespace std;
using namespace fasmio::fiber_env;

namespace {

    struct context_t {
        vector<int> numbers;
        CooperativeMutex lock;
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(1);
        context->lock.Lock();
        context->numbers.push_back(2);
        FiberEnv::Yield_s();
        context->numbers.push_back(4);
        FiberEnv::Yield_s();
        context->numbers.push_back(5);
        context->lock.Unlock();
        context->numbers.push_back(6);
        FiberEnv::Yield_s();
        context->numbers.push_back(9);

        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(3);
        context->lock.Lock();
        context->numbers.push_back(7);
        context->lock.Unlock();
        context->numbers.push_back(8);

        return 0;
    }

    TEST(CooperativeMutex, General) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber1, (void*)(&context), "fiber1"));
        EXPECT_TRUE(env.CreateThread(fiber2, (void*)(&context), "fiber2"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(9, context.numbers.size());
        for (int i=0; i<9; ++i)
            EXPECT_EQ(i+1, context.numbers[i]);
    }
}  // namespace <anonymous>

