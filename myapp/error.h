#pragma once

#include <jsoncpp/json/json.h>

#include <string>

namespace myapp
{
    class Error
    {
    public:

        using Args = std::map<std::string, std::string>;

        enum class Code
        {
            ExpectJsonBody,

            UsernameNotFound,
            UsernameInvalidSize,
            UsernameBeginWithDot,
            UsernameContainInvalidChar,

            PasswordNotFound,
            PassowrdInvalidSize,
            PasswordContainInvalidChar,

            UserAlreadyExist,
        };

        explicit Error(Code ec);

        Error(Code ec, const Args &args);

        virtual ~Error() = default;

        Code GetCode() const;

        const std::string &GetMessage() const;

        const Args &GetArgs() const;

        Json::Value GetJson() const;

    private:
        Code ec_;
        Args args_;
    };

} // namespace myapp
