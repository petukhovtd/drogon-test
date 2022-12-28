#pragma once

#include <string>
#include <memory>

namespace myapp
{
    using UserId = uint64_t;

    class User
    {
    public:
        explicit User(const std::string &login, const std::string &password, UserId id);

        virtual ~User() = default;

        const std::string &GetLogin() const;

        bool CheckPassword(const std::string &password) const;
        
        UserId GetId() const;

    private:
        std::string login_;
        std::string password_;
        UserId id_;
    };

    using UserPtr = std::shared_ptr<User>;
}
