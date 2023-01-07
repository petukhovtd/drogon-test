#pragma once

#include <myapp/user.h>

#include <string>
#include <unordered_map>

namespace myapp
{
    class UserSession
    {
    public:
        using Token = std::string;

        explicit UserSession() = default;

        virtual ~UserSession() = default;

        const Token &Create(const User &user);

        const Token &Update(const User &user);

        bool Check(UserId userId, const Token &token) const;

        void Delete(UserId userId);

        bool Autorizated(UserId userId) const;

    private:
        std::unordered_map<UserId, Token> db_;
    };

    using UserSessionPtr = std::shared_ptr<UserSession>;
}