
#include "gtest/gtest.h"

#include <vector>
#include "runtime-env/fiber-env/fiber-env.h"

using namespace std;
using namespace fasmio::runtime_env;
using namespace fasmio::fiber_env;

namespace test_sleep { namespace {

    struct context_t {
        vector<int> numbers;
        NativeMutex lock;
        void AddNumber(int n) {
            lock.Lock();
            numbers.push_back(n);
            lock.Unlock();
        }
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        FiberEnv::Sleep_s(5);
        context->AddNumber(2);
        FiberEnv::Sleep_s(10);
        context->AddNumber(4);

        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->AddNumber(1);
        FiberEnv::Sleep_s(10);
        context->AddNumber(3);
        FiberEnv::Sleep_s(10);
        context->AddNumber(5);

        return 0;
    }

    TEST(FiberEnv, Sleep) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber1, (void*)(&context), "fiber1"));
        EXPECT_TRUE(env.CreateThread(fiber2, (void*)(&context), "fiber2"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(5, context.numbers.size());
        EXPECT_EQ(1, context.numbers[0]);
        EXPECT_EQ(2, context.numbers[1]);
        EXPECT_EQ(3, context.numbers[2]);
        EXPECT_EQ(4, context.numbers[3]);
        EXPECT_EQ(5, context.numbers[4]);
    }

}}  // namespace test_sleep::<anonymous>

namespace test_yield { namespace {

    struct context_t {
        vector<int> numbers;
        NativeMutex lock;

        void AddNumber(int n) {
            lock.Lock();
            numbers.push_back(n);
            lock.Unlock();
        }
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        FiberEnv::Yield_s();
        context->AddNumber(2);
        FiberEnv::Yield_s();
        context->AddNumber(4);

        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->AddNumber(1);
        FiberEnv::Yield_s();
        context->AddNumber(3);
        FiberEnv::Yield_s();
        context->AddNumber(5);

        return 0;
    }

    TEST(FiberEnv, Yield) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber1, (void*)(&context), "fiber1"));
        EXPECT_TRUE(env.CreateThread(fiber2, (void*)(&context), "fiber2"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(5, context.numbers.size());
        EXPECT_EQ(1, context.numbers[0]);
        EXPECT_EQ(2, context.numbers[1]);
        EXPECT_EQ(3, context.numbers[2]);
        EXPECT_EQ(4, context.numbers[3]);
        EXPECT_EQ(5, context.numbers[4]);
    }

}}  // namespace test_yield::<anonymous>

namespace test_quit { namespace {

    struct context_t {
        vector<int> numbers;
        NativeMutex lock;

        void AddNumber(int n) {
            lock.Lock();
            numbers.push_back(n);
            lock.Unlock();
        }
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->AddNumber(1);
        FiberEnv::Quit_s(0);
        context->AddNumber(111);

        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->AddNumber(2);
        return 0;
    }

    TEST(FiberEnv, Quit) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber1, (void*)(&context), "fiber1"));
        EXPECT_TRUE(env.CreateThread(fiber2, (void*)(&context), "fiber2"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(2, context.numbers.size());
        EXPECT_EQ(1, context.numbers[0]);
        EXPECT_EQ(2, context.numbers[1]);
    }

}}  // namespace test_quit::<anonymous>

namespace test_join { namespace {

    struct context_t {
        vector<int> numbers;
        IThread* fiber1;
        IThread* fiber2;
        IThread* fiber3;
        IThread* fiber4;
    };

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(1);
        FiberEnv::Sleep_s(20);
        context->numbers.push_back(6);

        return 1;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(2);
        FiberEnv::Sleep_s(5);
        context->numbers.push_back(5);
        if (1 == context->fiber1->Join())
            context->numbers.push_back(7);
        else
            context->numbers.push_back(999);

        return 2;
    }

    int fiber3(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(3);
        if (2 == context->fiber2->Join())
            context->numbers.push_back(8);
        else
            context->numbers.push_back(999);
        if (4 == context->fiber4->Join())
            context->numbers.push_back(9);
        else
            context->numbers.push_back(999);

        return 3;
    }

    int fiber4(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);

        context->numbers.push_back(4);
        FiberEnv::Quit_s(4);

        return 0;
    }

    TEST(FiberEnv, Join) {
        FiberEnv env;

        context_t context;

        EXPECT_TRUE(env.Start(0));
        context.fiber1 = env.CreateThread(fiber1, (void*)(&context), "fiber1");
        context.fiber2 = env.CreateThread(fiber2, (void*)(&context), "fiber2");
        context.fiber3 = env.CreateThread(fiber3, (void*)(&context), "fiber3");
        context.fiber4 = env.CreateThread(fiber4, (void*)(&context), "fiber4");
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(9, context.numbers.size());
        for (int i=0; i<context.numbers.size(); ++i)
            EXPECT_EQ(i+1, context.numbers[i]);
    }

}}  // namespace test_sleep::<anonymous>

namespace test_self { namespace {

    struct context_t {
        int errors;
        vector<IThread*> fibers;
    };

    int fiber0(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        if (FiberEnv::CurrentThread_s() != context->fibers[0])
            ++(context->errors);
        return 0;
    }

    int fiber1(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        if (FiberEnv::CurrentThread_s() != context->fibers[1])
            ++(context->errors);
        return 0;
    }

    int fiber2(void* arg)
    {
        context_t *context = reinterpret_cast<context_t*>(arg);
        if (FiberEnv::CurrentThread_s() != context->fibers[2])
            ++(context->errors);
        return 0;
    }

    TEST(FiberEnv, Self) {
        FiberEnv env;

        context_t context;
        context.errors = 0;

        EXPECT_TRUE(env.Start(0));
        context.fibers.push_back(env.CreateThread(fiber0, (void*)(&context), "fiber0"));
        context.fibers.push_back(env.CreateThread(fiber1, (void*)(&context), "fiber1"));
        context.fibers.push_back(env.CreateThread(fiber2, (void*)(&context), "fiber2"));
        env.Stop();
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());

        EXPECT_EQ(0, context.errors);
    }

}}  // namespace test_sleep::<anonymous>

namespace test_fiberize { namespace {

    int fiber(void* arg)
    {
        FiberEnv::Sleep_s(10);
        return 0;
    }

    int fiber_main(void* arg)
    {
        FiberEnv::CreateThread_s(fiber, 0, "fiber");
        FiberEnv::Yield_s();
        FiberEnv::Stop_s();
        FiberEnv::Sleep_s(10);
        return 0;
    }

    TEST(FiberEnv, FiberizeThisThread) {
        FiberEnv env;

        EXPECT_TRUE(env.Start(0));
        EXPECT_TRUE(env.CreateThread(fiber_main, 0, "main"));
        EXPECT_TRUE(env.FiberizeThisThread());
        EXPECT_TRUE(env.Join());
    }

}}  // namespace test_fiberize::<anonymous>

namespace test_daemon { namespace {

    int fiber1(void* arg)
    {
        FiberEnv::Sleep_s(10);
        return 0;
    }

    int fiber2(void* arg)
    {
        return 0;
    }

    TEST(FiberEnv, FiberizeThisThread) {
        FiberEnv env;

        EXPECT_TRUE(env.Start(1));

        IThread* f1 = env.CreateThread(fiber1, 0, "f1");
        EXPECT_TRUE(f1 != nullptr);
        f1->SetDaemon();

        IThread* f2 = env.CreateThread(fiber2, 0, "f2");
        EXPECT_TRUE(f2 != nullptr);
        usleep(1000*10);
        f2->SetDaemon();

        env.Stop();
        EXPECT_TRUE(env.Join());
    }

}}  // namespace test_daemon::<anonymous>

