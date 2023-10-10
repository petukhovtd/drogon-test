#pragma once

#include <libapi/error.h>

#include <string_view>
#include <variant>

namespace myapp {

std::pair<std::string_view, std::string_view> SplitBy(std::string_view s, std::string_view delimiter);

namespace basic {

struct Credentials {
  std::string username;
  std::string password;
};

std::variant<Credentials, Error> GetCredentials(std::string_view authHeader);

std::string MakeAuth(const Credentials &credentials);

}
}
