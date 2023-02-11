#pragma once

#include <myapp/error.h>

#include <string>
#include <optional>
#include <variant>
#include <sstream>

namespace myapp
{
    std::variant<std::string, Error> GetUsername(const Json::Value &json);

    std::optional<Error> CheckUsername(const std::string &username);

    std::variant<std::string, Error> ExtractUsername(const Json::Value &json);

    std::variant<std::string, Error> GetPassword(const Json::Value &json);

    std::optional<Error> CheckPassword(const std::string &password);

    std::variant<std::string, Error> ExtractPassword(const Json::Value &json);

    template <typename T>
    std::optional<T> Extract(const std::string &val)
    {
        std::istringstream os(val);

        T result;
        os >> result;

        if (!os)
        {
            return std::nullopt;
        }

        return result;
    }
}