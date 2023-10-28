#pragma once

#include <string>

#include <drogon/drogon.h>

namespace myapp {

template<typename JsonType, typename ValueType>
bool GetTypeValueFromJson(const Json::Value &json, const std::string &key, ValueType &result) {
  const auto *value = json.find(key.data(), key.data() + key.length());
  if (!value) {
    return false;
  }
  if (!value->is<JsonType>()) {
    return false;
  }
  result = value->as<ValueType>();
  return true;
}

template<typename JsonType, typename ValueType>
bool GetTypeOptValueFromJson(const Json::Value &json, const std::string &key, std::optional<ValueType> &result) {
  const auto *value = json.find(key.data(), key.data() + key.length());
  if (!value) {
    return true;
  }
  if (!value->is<JsonType>()) {
    return false;
  }
  result = value->as<ValueType>();
  return true;
}

template<typename ValueType>
bool GetValueFromJson(const Json::Value &json, const std::string &key, ValueType &result) {
  return GetTypeValueFromJson<ValueType, ValueType>(json, key, result);
}

template<typename ValueType>
bool GetOptValueFromJson(const Json::Value &json, const std::string &key, std::optional<ValueType> &result) {
  return GetTypeOptValueFromJson<ValueType, ValueType>(json, key, result);
}

struct ServerAddress {
  std::string ip;
  uint16_t port;
};

ServerAddress GetServerAddress();

drogon::HttpClientPtr MakeHttpClient(const ServerAddress &serverAddress);

drogon::HttpClientPtr MakeHttpClient();

struct UserMainInfo {
  std::string username;
  std::string password;
  uint64_t id;
};

bool CreateUser(UserMainInfo &userMainInfo);

bool DeleteUser(const UserMainInfo &userMainInfo);

struct UserListInfo {
  size_t size;
  size_t total;
  std::optional<size_t> limit;
  std::optional<size_t> offset;
  std::vector<UserMainInfo> users;
};

UserListInfo GetList(std::optional<uint64_t> limit, std::optional<uint64_t> offset);

}