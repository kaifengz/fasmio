
#include "gtest/gtest.h"

#include "runtime-env/fiber-env/cooperative-event.h"
#include "runtime-env/fiber-env/cooperative-mutex.h"
#include "runtime-env/fiber-env/fiber-env.h"

using namespace std;
using namespace fasmio::fiber_env;

namespace test_wait { namespace {

    struct context_t {
        vector<int> numbers;
        CooperativeEvent not_empty_event;
        CooperativeEvent not_full_event;

        context_t() : numbers(), not_empty_event(false), not_full_event(true)
        {
        }
        void fake_produce() {};
        void fake_consume() {};
    };

    int fiber_producer(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->numbers.push_back(1);

        context->not_full_event.Wait();
        context->numbers.push_back(2);
        context->fake_produce();
        context->not_full_event.Reset();
        context->numbers.push_back(3);
        context->not_empty_event.Set();
        context->numbers.push_back(4);

        context->not_full_event.Wait();
        context->numbers.push_back(9);
        context->fake_produce();
        context->not_full_event.Reset();
        context->numbers.push_back(10);
        context->not_empty_event.Set();
        context->numbers.push_back(11);

        return 0;
    }

    int fiber_consumer(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        context->numbers.push_back(5);

        context->not_empty_event.Wait();
        context->numbers.push_back(6);
        context->fake_consume();
        context->not_empty_event.Reset();
        context->numbers.push_back(7);
        context->not_full_event.Set();
        context->numbers.push_back(8);

        context->not_empty_event.Wait();
        context->numbers.push_back(12);
        context->fake_consume();
        context->not_empty_event.Reset();
        context->numbers.push_back(13);
        context->not_full_event.Set();
        context->numbers.push_back(14);

        return 0;
    }

    TEST(CooperativeEvent, Wait) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber_producer, (void*)(&context), "producer"));
        EXPECT_TRUE(env.CreateThread(fiber_consumer, (void*)(&context), "consumer"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(14, context.numbers.size());
        for (int i=0; i<14; ++i)
            EXPECT_EQ(i+1, context.numbers[i]);
    }
}}  // namespace test_general::<anonymous>

namespace test_timedwait { namespace {

    struct context_t {
        vector<int> numbers;
        CooperativeEvent event;

        context_t() : numbers(), event()
        {
        }
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(1);
        if (context->event.TimedWait(5))
            context->numbers.push_back(999);
        else
            context->numbers.push_back(3);
        if (context->event.TimedWait(10))
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

        context->event.Set();
        context->numbers.push_back(5);

        return 0;
    }

    TEST(CooperativeEvent, TimedWait) {
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

    // TODO: test_racecondition of CooperativeEvent

}}  // namespace test_racecondition::<anonymous>

