#include "user.h"

#include <myapp/misc.h>

namespace myapp
{

    User::User(const std::string &login, const std::string &password, UserId id)
        : login_(login), password_(CalculateMd5(password)), id_(id)
    {
    }

    const std::string &User::GetLogin() const
    {
        return login_;
    }

    const std::string& User::GetPasswordHash() const
    {
        return password_;
    }

    bool User::CheckPassword(const std::string &password) const
    {
        return password_ == CalculateMd5(password);
    }

    UserId User::GetId() const
    {
        return id_;
    }

}