#pragma once

#include <myapp/user.h>

#include <unordered_map>

namespace myapp
{
    class UserDb
    {
        using UserWeak = std::weak_ptr<UserPtr::element_type>;

    public:
        explicit UserDb();

        virtual ~UserDb() = default;

        UserPtr AddUser(const std::string &login, const std::string &password);

        UserPtr GetUser(UserId id) const;

        UserPtr GetUser(const std::string &login) const;

    private:
        std::unordered_map<UserId, UserPtr> db_;
        std::unordered_map<std::string, UserWeak> loginIndex_;
        UserId lastId_;
    };

    using UserDbPtr = std::shared_ptr< UserDb >;
}
