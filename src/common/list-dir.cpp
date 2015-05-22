
#include "./list-dir.h"
#include <dirent.h>
#include <unistd.h>

namespace fasmio { namespace common {

ListDir::ListDir(const char* root, unsigned int flags) :
    flags_(flags),
    root_(),
    curr_(),
    dir_(nullptr)
{
    if (root != nullptr)
    {
        root_ = root;
        dir_ = reinterpret_cast<void*>(opendir(root));
    }
}

ListDir::~ListDir()
{
    if (dir_ != nullptr)
        closedir(reinterpret_cast<DIR*>(dir_));
}

bool ListDir::Next(std::string *fullname, std::string *shortname, ItemType *type)
{
    if (dir_ == nullptr)
        return false;

    while (true)
    {
        struct dirent d, *pd = nullptr;
        if (0 != readdir_r(reinterpret_cast<DIR*>(dir_), &d, &pd))
            return false;
        if (pd == nullptr)
            return false;

        if (((d.d_type & DT_DIR) == 0 && (flags_ & FILES) != 0) ||
            ((d.d_type & DT_DIR) != 0 && (flags_ & DIRS ) != 0))
        {
            std::string name(d.d_name);
            if ((flags_ & OMIT_DOT) != 0 && (name == "." || name == ".."))
                continue;

            if (fullname != nullptr)
                *fullname = root_ + "/" + name;
            if (shortname != nullptr)
                *shortname = name;
            if (type != nullptr)
                *type = (d.d_type & DT_DIR) == 0 ? T_FILE : T_DIR;
            return true;
        }
    }
}

}}  // namespace fasmio::common
