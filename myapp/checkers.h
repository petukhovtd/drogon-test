#pragma once

#include <myapp/error.h>

#include <string>
#include <optional>
#include <variant>

namespace myapp
{
    std::variant<std::string, Error> GetUsername(const Json::Value &json);

    std::optional<Error> CheckUsername(const std::string &username);

    std::variant<std::string, Error> ExtractUsername(const Json::Value &json);

    std::variant<std::string, Error> GetPassword(const Json::Value &json);
    
    std::optional<Error> CheckPassword(const std::string &password);

    std::variant<std::string, Error> ExtractPassword(const Json::Value &json);
}