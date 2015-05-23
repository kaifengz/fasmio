
#include "./uuid.h"
#include <uuid/uuid.h>

namespace fasmio { namespace common {

std::string GenUUID()
{
    uuid_t uuid;
    char str[40];
    uuid_generate(uuid);
    uuid_unparse(uuid, str);
    return std::string(str);
}

bool GenUUID(char* buff, long buff_size)
{
    if (buff == nullptr || buff_size <= 36)
        return false;

    uuid_t uuid;
    uuid_generate(uuid);
    uuid_unparse(uuid, buff);
    return true;
}

}}  // namespace fasmio::common

