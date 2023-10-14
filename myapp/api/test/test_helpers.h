#pragma once

#include <string>

#include <drogon/drogon.h>

namespace myapp {

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

}