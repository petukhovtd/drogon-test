#include <user/user.h>

#include <user/misc.h>

namespace myapp
{

    User::User(const std::string &username, const std::string &password, UserId id)
        : username_(username), password_(CalculateMd5(password)), id_(id), info_()
    {
    }

    const std::string &User::GetUsername() const
    {
        return username_;
    }

    const std::string &User::GetPasswordHash() const
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

    const User::Info &User::GetInfo() const
    {
        return info_;
    }

    void User::SetInfo(const Info &&info)
    {
        info_ = std::move(info);
    }
}