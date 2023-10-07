#include <myapp/error.h>

#include <unordered_map>

namespace myapp {
Error::Error(Code ec)
    : ec_(ec) {
}

Error::Error(Code ec, const Args &args)
    : ec_(ec), args_(args) {
}

Error::Code Error::GetCode() const {
  return ec_;
}

const std::string &Error::GetMessage() const {
  static const std::unordered_map<Code, std::string> messageMap{
      {Code::ExpectJsonBody, "expect json body"},

      {Code::ConvertParameterFailed, "convert parameter failed"},

      {Code::KeyNotFound, "key not found"},
      {Code::InvalidType, "invalid type"},
      {Code::InvalidValue, "invalid value"},

      {Code::UserAlreadyExist, "user already exist"},

      {Code::AuthorizateFailed, "authorizate failed"},
  };

  const auto it = messageMap.find(ec_);
  if (it == messageMap.end()) {
    static std::string empty = "unknown error";
    return empty;
  }

  return it->second;
}

const Error::Args &Error::GetArgs() const {
  return args_;
}

Json::Value Error::GetJson() const {
  Json::Value json;

  json["code"] = static_cast<int>(ec_);
  json["message"] = GetMessage();

  if (!args_.empty()) {
    Json::Value args;
    for (auto &&[key, value] : args_) {
      args[key] = value;
    }
    json["args"] = args;
  }

  return json;
}
}