
#include "gtest/gtest.h"

#include "runtime-env/fiber-env/cooperative-condition.h"
#include "runtime-env/fiber-env/cooperative-mutex.h"
#include "runtime-env/fiber-env/fiber-env.h"

using namespace std;
using namespace fasmio::fiber_env;

namespace test_wait { namespace {

    struct context_t {
        vector<int> numbers;
        CooperativeMutex lock;
        CooperativeCondition cond;

        context_t() : numbers(), lock(), cond(&lock)
        {
        }
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(1);
        context->cond.Acquire();
        context->numbers.push_back(2);
        FiberEnv::Yield_s();
        context->numbers.push_back(4);
        context->cond.Wait();
        context->numbers.push_back(8);
        context->cond.Release();
        context->numbers.push_back(9);

        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(3);
        context->cond.Acquire();
        context->numbers.push_back(5);
        context->cond.Signal();
        context->numbers.push_back(6);
        context->cond.Release();
        context->numbers.push_back(7);

        return 0;
    }

    TEST(CooperativeCondition, Wait) {
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
}}  // namespace test_general::<anonymous>

namespace test_timedwait { namespace {

    struct context_t {
        vector<int> numbers;
        CooperativeMutex lock;
        CooperativeCondition cond;

        context_t() : numbers(), lock(), cond(&lock)
        {
        }
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(1);
        context->cond.Acquire();
        context->numbers.push_back(2);
        if (context->cond.TimedWait(5))
            context->numbers.push_back(999);
        else
            context->numbers.push_back(4);
        if (context->cond.TimedWait(10))
            context->numbers.push_back(9);
        else
            context->numbers.push_back(999);
        context->cond.Release();
        context->numbers.push_back(10);

        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(3);
        FiberEnv::Sleep_s(10);
        context->numbers.push_back(5);

        context->cond.Acquire();
        context->numbers.push_back(6);
        context->cond.Signal();
        context->numbers.push_back(7);
        context->cond.Release();
        context->numbers.push_back(8);

        return 0;
    }

    TEST(CooperativeCondition, TimedWait) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber1, (void*)(&context), "fiber1"));
        EXPECT_TRUE(env.CreateThread(fiber2, (void*)(&context), "fiber2"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(10, context.numbers.size());
        for (int i=0; i<10; ++i)
            EXPECT_EQ(i+1, context.numbers[i]);
    }

}}  // namespace test_timedwait::<anonymous>

namespace test_racecondition { namespace {

    // TODO: test_racecondition of CooperativeCondition

}}  // namespace test_racecondition::<anonymous>

