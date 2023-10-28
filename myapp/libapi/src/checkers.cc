#include <libapi/checkers.h>
#include <libapi/api_keys.h>

#include <algorithm>
#include <sstream>

namespace myapp {

const Json::Value *FindKey(const std::string &key, const Json::Value &json) {
  return json.find(key.data(), key.data() + key.length());
}

std::variant<std::string, Error> GetString(const std::string &key, const Json::Value &json) {
  const auto *val = FindKey(key, json);
  if (!val) {
    return Error(Error::Code::KeyNotFound,
                 {{"key", key}});
  }

  if (!val->isString()) {
    return Error(Error::Code::InvalidType, {{"key", key},
                                            {"type", "string"}});
  }

  return val->asString();
}

std::optional<Error> CheckUsername(const std::string &username) {
  static const size_t minUsernameSize = 5;
  static const size_t maxUsernameSize = 128;

  if (username.size() < minUsernameSize || maxUsernameSize < username.size()) {
    return Error(Error::Code::InvalidValue,
                 {
                     {"key", key::username},
                     {"property", "size"},
                     {"min", std::to_string(minUsernameSize)},
                     {"max", std::to_string(maxUsernameSize)},
                     {"current", std::to_string(username.size())},
                 });
  }

  if (*username.begin() == '.') {
    return Error(Error::Code::InvalidValue, {
        {"key", key::username},
        {"property", "begin with dot"},
    });
  }

  const auto it = std::find_if_not(username.begin(),
                                   username.end(),
                                   [](unsigned char c) { return std::isalpha(c) || std::isdigit(c) || c == '.'; });
  if (it != username.end()) {
    std::ostringstream os;
    os << std::hex << static_cast<int>(*it);
    return Error(Error::Code::InvalidValue,
                 {
                     {"key", key::username},
                     {"property", "contain invalid char"},
                     {"pos", std::to_string(std::distance(username.begin(), it))},
                     {"hex", os.str()},
                 });
  }

  return std::nullopt;
}

std::variant<std::string, Error> ExtractUsername(const Json::Value &json) {
  const auto usernameOrError = GetString(key::username, json);
  if (std::holds_alternative<std::string>(usernameOrError)) {
    std::string username = std::get<std::string>(usernameOrError);

    const auto checkResult = CheckUsername(username);
    if (checkResult.has_value()) {
      return checkResult.value();
    }

    return username;
  } else {
    return std::get<Error>(usernameOrError);
  }
}

std::optional<Error> CheckPassword(const std::string &password) {
  static const size_t minPasswordSize = 8;
  static const size_t maxPasswordSize = 255;

  if (password.size() < minPasswordSize || maxPasswordSize < password.size()) {
    return Error(Error::Code::InvalidValue,
                 {
                     {"key", key::password},
                     {"property", "size"},
                     {"min", std::to_string(minPasswordSize)},
                     {"max", std::to_string(maxPasswordSize)},
                     {"current", std::to_string(password.size())},
                 });
  }

  const auto it = std::find_if_not(password.begin(), password.end(), [](unsigned char c) { return std::isgraph(c); });
  if (it != password.end()) {
    std::ostringstream os;
    os << std::hex << static_cast<int>(*it);
    return Error(Error::Code::InvalidValue,
                 {
                     {"key", key::password},
                     {"property", "contain invalid char"},
                     {"pos", std::to_string(std::distance(password.begin(), it))},
                     {"hex", os.str()},
                 });
  }

  return std::nullopt;
}

std::variant<std::string, Error> ExtractPassword(const Json::Value &json) {
  const auto passwordOrError = GetString(key::password, json);
  if (std::holds_alternative<std::string>(passwordOrError)) {
    std::string password = std::get<std::string>(passwordOrError);

    const auto checkResult = CheckPassword(password);
    if (checkResult.has_value()) {
      return checkResult.value();
    }

    return password;
  } else {
    return std::get<Error>(passwordOrError);
  }
}

} // namespace myapp
