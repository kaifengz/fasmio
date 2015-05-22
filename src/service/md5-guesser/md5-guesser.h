
#ifndef SERVICE_MD5_GUESSER_MD5_GUESSER_H_
#define SERVICE_MD5_GUESSER_MD5_GUESSER_H_

#include "service/sdk/service-base.h"
#include "service/sdk/general-xsd-task.h"
#include "./data-types.xsd.h"

namespace fasmio { namespace service { namespace md5_guesser {

typedef sdk::GeneralXsdTask<request_t>     GuessRequestTask;
typedef sdk::GeneralXsdTask<answer_t>      GuessAnswerTask;

class Md5Guesser : public sdk::ServiceBase
{
public:
    Md5Guesser(const char* name, IServiceAgent *agent, IRuntimeEnv* env, ILogger *logger);

public:
    virtual bool OnInitialize ();
    virtual void OnFinalize   ();

private:
    void Guess(std::unique_ptr<GuessRequestTask> task);
    bool VerifyMd5(const unsigned char* candidate, unsigned int length, const char* md5);

private:
    IOutQueue* answer_out_queue_;
};

}}}  // namespace fasmio::service::md5_guesser

#endif  // SERVICE_MD5_GUESSER_MD5_GUESSER_H_

