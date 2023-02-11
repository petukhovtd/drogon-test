#include <myapp/checkers.h>

#include <algorithm>
#include <sstream>

namespace myapp
{
    std::optional<std::string> GetUsername(const Json::Value &json)
    {
        if (!json.isMember("username"))
        {
            return std::nullopt;
        }

        return json.get("username", "").asString();
    }

    std::optional<Error> CheckUsername(const std::string &username)
    {
        static const size_t minUsernameSize = 5;
        static const size_t maxUsernameSize = 128;

        if (username.size() < minUsernameSize || maxUsernameSize < username.size())
        {
            return Error(Error::Code::UsernameInvalidSize,
                         {
                             {"min", std::to_string(minUsernameSize)},
                             {"max", std::to_string(maxUsernameSize)},
                             {"current", std::to_string(username.size())},
                         });
        }

        if (*username.begin() == '.')
        {
            return Error(Error::Code::UsernameBeginWithDot);
        }

        const auto it = std::find_if_not(username.begin(), username.end(), [](unsigned char c)
                                         { return std::isalpha(c) || std::isdigit(c) || c == '.'; });
        if (it != username.end())
        {
            std::ostringstream os;
            os << std::hex << static_cast<int>(*it);
            return Error(Error::Code::UsernameContainInvalidChar,
                         {
                             {"pos", std::to_string(std::distance(username.begin(), it))},
                             {"hex", os.str()},
                         });
        }

        return std::nullopt;
    }

    std::variant<std::string, Error> ExtractUsername(const Json::Value &json)
    {
        const auto usernameOpt = GetUsername(json);
        if (!usernameOpt.has_value())
        {
            return Error(Error::Code::UsernameNotFound);
        }
        const auto checkResult = CheckUsername(usernameOpt.value());
        if (checkResult.has_value())
        {
            return checkResult.value();
        }

        return usernameOpt.value();
    }

    std::optional<std::string> GetPassword(const Json::Value &json)
    {
        if (!json.isMember("password"))
        {
            return std::nullopt;
        }

        return json.get("password", "").asString();
    }

    std::optional<Error> CheckPassword(const std::string &password)
    {
        static const size_t minPasswordSize = 8;
        static const size_t maxPasswordSize = 255;

        if (password.size() < minPasswordSize || maxPasswordSize < password.size())
        {
            return Error(Error::Code::PassowrdInvalidSize,
                         {
                             {"min", std::to_string(minPasswordSize)},
                             {"max", std::to_string(maxPasswordSize)},
                             {"current", std::to_string(password.size())},
                         });
        }

        const auto it = std::find_if_not(password.begin(), password.end(), [](unsigned char c)
                                         { return std::isgraph(c); });
        if (it != password.end())
        {
            std::ostringstream os;
            os << std::hex << static_cast<int>(*it);
            return Error(Error::Code::PasswordContainInvalidChar,
                         {
                             {"pos", std::to_string(std::distance(password.begin(), it))},
                             {"hex", os.str()},
                         });
        }

        return std::nullopt;
    }

    std::variant<std::string, Error> ExtractPassword(const Json::Value &json)
    {
        const auto passwordOpt = GetPassword(json);
        if (!passwordOpt.has_value())
        {
            return Error(Error::Code::PasswordNotFound);
        }
        const auto checkResult = CheckPassword(passwordOpt.value());
        if (checkResult.has_value())
        {
            return checkResult.value();
        }

        return passwordOpt.value();
    }
} // namespace myapp
