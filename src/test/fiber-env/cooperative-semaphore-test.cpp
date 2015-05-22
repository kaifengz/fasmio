
#include "gtest/gtest.h"

#include "runtime-env/fiber-env/cooperative-mutex.h"
#include "runtime-env/fiber-env/cooperative-semaphore.h"
#include "runtime-env/fiber-env/fiber-env.h"

using namespace std;
using namespace fasmio::fiber_env;

namespace test_wait { namespace {

    struct context_t {
        vector<int> numbers;
        CooperativeSemaphore semaphore;

        context_t() : numbers(), semaphore(2, 2)
        {
        }
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->numbers.push_back(1);

        context->semaphore.Wait();
        context->numbers.push_back(2);
        FiberEnv::Yield_s();

        context->numbers.push_back(6);
        context->semaphore.Release();
        context->numbers.push_back(7);
        FiberEnv::Yield_s();

        context->numbers.push_back(11);
        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->numbers.push_back(3);

        context->semaphore.Wait();
        context->numbers.push_back(4);
        FiberEnv::Yield_s();

        context->numbers.push_back(8);
        FiberEnv::Yield_s();

        context->numbers.push_back(12);
        context->semaphore.Release();
        context->numbers.push_back(13);
        return 0;
    }

    int fiber3(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->numbers.push_back(5);

        context->semaphore.Wait();
        context->numbers.push_back(9);
        context->semaphore.Release();
        context->numbers.push_back(10);
        return 0;
    }

    TEST(CooperativeSemaphore, Wait) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber1, (void*)(&context), "fiber1"));
        EXPECT_TRUE(env.CreateThread(fiber2, (void*)(&context), "fiber2"));
        EXPECT_TRUE(env.CreateThread(fiber3, (void*)(&context), "fiber3"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(13, context.numbers.size());
        for (int i=0; i<13; ++i)
            EXPECT_EQ(i+1, context.numbers[i]);
    }
}}  // namespace test_general::<anonymous>

namespace test_timedwait { namespace {

    struct context_t {
        vector<int> numbers;
        CooperativeSemaphore semaphore;

        context_t() : numbers(), semaphore(1, 0)
        {
        }
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(1);
        if (context->semaphore.TimedWait(5))
            context->numbers.push_back(999);
        else
            context->numbers.push_back(3);
        if (context->semaphore.TimedWait(10))
            context->numbers.push_back(6);
        else
            context->numbers.push_back(999);

        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(2);
        FiberEnv::Sleep_s(10);
        context->numbers.push_back(4);

        context->semaphore.Release();
        context->numbers.push_back(5);

        return 0;
    }

    TEST(CooperativeSemaphore, TimedWait) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber1, (void*)(&context), "fiber1"));
        EXPECT_TRUE(env.CreateThread(fiber2, (void*)(&context), "fiber2"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(6, context.numbers.size());
        for (int i=0; i<6; ++i)
            EXPECT_EQ(i+1, context.numbers[i]);
    }

}}  // namespace test_timedwait::<anonymous>

namespace test_racecondition { namespace {

    // TODO: test_racecondition of CooperativeSemaphore

}}  // namespace test_racecondition::<anonymous>

