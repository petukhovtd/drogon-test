#pragma once

#include <string>
#include <memory>

namespace myapp
{
    using UserId = uint64_t;

    class User
    {
    public:
        struct Info
        {
            std::string firstName;
            std::string lastName;
        };

        explicit User(const std::string &username, const std::string &password, UserId id);

        virtual ~User() = default;

        const std::string &GetUsername() const;

        const std::string &GetPasswordHash() const;

        bool CheckPassword(const std::string &password) const;

        UserId GetId() const;

        const Info &GetInfo() const;

        void SetInfo(const Info &&info);

    private:
        std::string username_;
        std::string password_;
        UserId id_;
        Info info_;
    };

    using UserPtr = std::shared_ptr<User>;
}
