
#ifndef COMMON_UUID_H_
#define COMMON_UUID_H_

#include <string>

namespace fasmio { namespace common {

std::string GenUUID();
bool GenUUID(char* buff, long buff_size);

}}  // namespace fasmio::common

#endif  // COMMON_UUID_H_

