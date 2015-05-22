
#include "./md5-guesser.h"
#include <string.h>

static const char* default_charset =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?";

namespace fasmio { namespace service { namespace md5_guesser {

REGISTER_SERVICE("md5-guesser", Md5Guesser);

Md5Guesser::Md5Guesser(const char* name, IServiceAgent *agent, IRuntimeEnv* env, ILogger *logger) :
    ServiceBase(name, agent, env, logger),
    answer_out_queue_(nullptr)
{
}

bool Md5Guesser::OnInitialize()
{
    if (!RegisterPushSlot<Md5Guesser, GuessRequestTask, &Md5Guesser::Guess>("guess", &GuessRequestTask::Allocator, 4))
        return false;
    if (!RegisterPopSlot("answer", &answer_out_queue_))
        return false;

    return true;
}

void Md5Guesser::OnFinalize()
{
}

void Md5Guesser::Guess(std::unique_ptr<GuessRequestTask> task)
{
    const unsigned int kMaxGuessLength = 32;

    request_t &req = **task;
    const unsigned int min_length = req.prefix.size();
    const unsigned int max_length = req.max_length;
    if (min_length > max_length || max_length > kMaxGuessLength)
        return;

    std::string charset;
    if (req.charset.get() != nullptr)
        charset.swap(*req.charset);
    else
        charset = default_charset;

    unsigned char candidate[kMaxGuessLength];
    unsigned int length = req.prefix.size();

    if (length > 0)
        memcpy(candidate, req.prefix.c_str(), length);

    const unsigned char first_char = charset[0];
    unsigned char next_char_table[256];

    memset(next_char_table, 0, sizeof(next_char_table));
    for (unsigned int i=1; i<charset.size(); ++i)
    {
        unsigned int c = charset[i-1];
        next_char_table[c] = charset[i];
    }

    std::unique_ptr<GuessAnswerTask> answer(new GuessAnswerTask());
    (**answer).md5 = req.md5;

    while (true)
    {
        if (VerifyMd5(candidate, length, req.md5.c_str()))
            (**answer).answers.push_back(std::string(reinterpret_cast<char*>(candidate), length));

        if (length < max_length)
        {
            candidate[length] = first_char;
            ++length;
            continue;
        }

        while (length > min_length)
        {
            unsigned char next = next_char_table[candidate[length-1]];
            if (next != 0)
            {
                candidate[length-1] = next;
                break;
            }
            --length;
        }

        if (length <= min_length)
            break;
    }

    answer_out_queue_->AddTask(answer.release());
}

bool Md5Guesser::VerifyMd5(const unsigned char* candidate, unsigned int length, const char* md5)
{
    // TODO: calculate the MD5 of candidate
    return false;
}

}}}  // namespace fasmio::service::md5_guesser

