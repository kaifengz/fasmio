
#ifndef COMMON_LIST_DIR_H_
#define COMMON_LIST_DIR_H_

#include <string>

namespace fasmio { namespace common {

class ListDir
{
public:
    enum
    {
        OMIT_DOT = 0x01,
        FILES    = 0x02,
        DIRS     = 0x04,
        DEFAULT_FLAGS = OMIT_DOT | FILES | DIRS,
    };

    explicit ListDir(const char* root, unsigned int flags = DEFAULT_FLAGS);
    ~ListDir();

public:
    enum ItemType
    {
        T_FILE = 0,
        T_DIR,
    };
    bool Next(std::string *fullname, std::string *shortname, ItemType *type);

private:
    const unsigned int flags_;
    std::string root_;
    std::string curr_;
    void* dir_;
};

}}  // namespace fasmio::common

#endif  // COMMON_LIST_DIR_H_

