
#include "./platform-thread.h"

namespace fasmio { namespace pthread_env {

pthread_key_t PlatformThread::key_;
bool PlatformThread::key_initialized_ = false;
pthread_once_t PlatformThread::key_once_ = PTHREAD_ONCE_INIT;

PlatformThread::PlatformThread(void (*func)(void*), void* arg, unsigned long stack_size) :
    func_(func), arg_(arg), stack_size_(stack_size), thread_(0)
{
}

PlatformThread::~PlatformThread()
{
}

bool PlatformThread::Start()
{
    if (thread_ != 0)
        return true;

    pthread_attr_t attr;
    if (0 != pthread_attr_init(&attr))
        return false;
    if (stack_size_ != 0)
        pthread_attr_setstacksize(&attr, stack_size_);

    int ret = pthread_create(&thread_, &attr, ThreadProc, static_cast<void*>(this));
    pthread_attr_destroy(&attr);

    return ret == 0 ? true : false;
}

bool PlatformThread::Join()
{
    if (thread_ == 0)
        return false;

    int ret = pthread_join(thread_, nullptr);
    thread_ = 0;
    return ret == 0;
}

void* PlatformThread::GetSpecific()
{
    if (!key_initialized_)
        pthread_once(&key_once_, InitKey);

    return pthread_getspecific(key_);
}

bool PlatformThread::SetSpecific(void* value)
{
    if (!key_initialized_)
        pthread_once(&key_once_, InitKey);

    return 0 == pthread_setspecific(key_, value);
}

void* PlatformThread::ThreadProc(void* arg)
{
    PlatformThread* thread = reinterpret_cast<PlatformThread*>(arg);
    if (thread != 0)
        (thread->func_)(thread->arg_);
    return nullptr;
}

void PlatformThread::InitKey()
{
    pthread_key_create(&key_, nullptr);
    key_initialized_ = true;
}

}}  // namespace fasmio::pthread_env

