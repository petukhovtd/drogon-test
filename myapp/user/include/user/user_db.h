#pragma once

#include <user/user.h>

#include <unordered_map>

namespace myapp
{
    class UserDb
    {
        using UserWeak = std::weak_ptr<UserPtr::element_type>;

    public:
        using UserDbType = std::unordered_map<UserId, UserPtr>;
        using UserDbConstIt = UserDbType::const_iterator;
        using UserDbIt = UserDbType::iterator;

        explicit UserDb();

        virtual ~UserDb() = default;

        UserPtr AddUser(const std::string &username, const std::string &password);

        UserPtr GetUser(UserId id) const;

        UserPtr GetUser(const std::string &username) const;

        UserDbConstIt begin() const;

        UserDbConstIt end() const;

        UserDbIt begin();

        UserDbIt end();

        size_t size() const;

    private:
        std::unordered_map<UserId, UserPtr> db_;
        std::unordered_map<std::string, UserWeak> loginIndex_;
        UserId lastId_;
    };

    using UserDbPtr = std::shared_ptr< UserDb >;
}
