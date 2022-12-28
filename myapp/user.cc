#include "user.h"

namespace myapp
{

    User::User(const std::string &login, const std::string &password, UserId id)
        : login_(login), password_(password), id_(id)
    {
    }

    const std::string &User::GetLogin() const
    {
        return login_;
    }

    bool User::CheckPassword(const std::string &password) const
    {
        return password_ == password;
    }

    UserId User::GetId() const
    {
        return id_;
    }

}