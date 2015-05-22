
#include "gtest/gtest.h"

#include "runtime-env/fiber-env/cooperative-rwlock.h"
#include "runtime-env/fiber-env/fiber-env.h"
#include "runtime-env/interlocked.h"

using namespace std;
using namespace fasmio::runtime_env;
using namespace fasmio::fiber_env;

namespace test_general { namespace {

    struct context_t {
        vector<int> numbers;
        CooperativeRWLock rwlock;

        void append(int n)
        {
            numbers.push_back(n);
        }
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->append(1);
        context->rwlock.ReadLock();
            context->append(2);
            FiberEnv::Yield_s();
            context->append(6);
        context->rwlock.Unlock();
        context->append(7);
        context->rwlock.ReadLock();
            context->append(14);
            FiberEnv::Yield_s();
            context->append(15);
        context->rwlock.Unlock();
        context->append(16);

        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->append(3);
        context->rwlock.ReadLock();
            context->append(4);
            FiberEnv::Yield_s();
            context->append(8);
            FiberEnv::Yield_s();
            context->append(9);
        context->rwlock.Unlock();
        context->append(10);
        context->rwlock.WriteLock();
            context->append(17);
        context->rwlock.Unlock();
        context->append(18);

        return 0;
    }

    int fiber3(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->append(5);
        context->rwlock.WriteLock();
            context->append(11);
            FiberEnv::Yield_s();
            context->append(12);
        context->rwlock.Unlock();
        context->append(13);

        return 0;
    }

    TEST(CooperativeRWLock, General) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber1, (void*)(&context), "fiber1"));
        EXPECT_TRUE(env.CreateThread(fiber2, (void*)(&context), "fiber2"));
        EXPECT_TRUE(env.CreateThread(fiber3, (void*)(&context), "fiber3"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(18, context.numbers.size());
        for (int i=0; i<18; ++i)
            EXPECT_EQ(i+1, context.numbers[i]);
    }
}}  // namespace test_general::<anonymous>

namespace test_racecondition { namespace {

    struct context_t
    {
        CooperativeRWLock rwlock;
        long iterations;
        long errors;
        long readers;
        long writers;
    };

    int fiber(void* arg)
    {
#define RANDOM()  (rand() % 1000 / 1000.0)

        context_t *context = reinterpret_cast<context_t*>(arg);

        for (int i=0; i<context->iterations; ++i)
        {
            if (RANDOM() < 0.8)
            {
                context->rwlock.ReadLock();
                if (interlocked::Increment(&context->readers) <= 0)
                    interlocked::Increment(&context->errors);
                else if (context->writers != 0)
                    interlocked::Increment(&context->errors);
                interlocked::Decrement(&context->readers);
                context->rwlock.Unlock();
            }
            else
            {
                context->rwlock.WriteLock();
                if (interlocked::Increment(&context->writers) != 1)
                    interlocked::Increment(&context->errors);
                else if (context->readers != 0)
                    interlocked::Increment(&context->errors);
                interlocked::Decrement(&context->writers);
                context->rwlock.Unlock();
            }
        }

        return 0;
    }

    TEST(CooperativeRWLock, RaceCondition) {
        FiberEnv env;

        context_t context;
        context.iterations = 1000 * 50;
        context.errors     = 0;
        context.readers    = 0;
        context.writers    = 0;

        for (int i=0; i<4; ++i)
            EXPECT_TRUE(env.CreateThread(fiber, (void*)(&context), "fiber"));
        EXPECT_TRUE(env.Start(8));
        env.Stop();
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(0, context.errors);
        EXPECT_EQ(0, context.readers);
        EXPECT_EQ(0, context.writers);
    }
}}  // namespace test_racecondition::<anonymous>

