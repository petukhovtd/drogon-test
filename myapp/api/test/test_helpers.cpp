#include "test_helpers.h"

#include <libapi/auth.h>
#include <libapi/api_keys.h>
#include <libapi/checkers.h>

namespace myapp {
ServerAddress GetServerAddress() {
  return {"127.0.0.1", 3000};
}

drogon::HttpClientPtr MakeHttpClient(const ServerAddress &serverAddress) {
  static auto client = drogon::HttpClient::newHttpClient(serverAddress.ip, serverAddress.port);
  return client;
}

drogon::HttpClientPtr MakeHttpClient() {
  return MakeHttpClient(GetServerAddress());
}

bool CreateUser(UserMainInfo &userMainInfo) {
  auto client = MakeHttpClient();

  Json::Value body;
  body[myapp::key::username] = userMainInfo.username;
  body[myapp::key::password] = userMainInfo.password;

  auto req = drogon::HttpRequest::newHttpJsonRequest(body);
  req->setMethod(drogon::Post);
  req->setPath("/api/v1/user");

  const auto response = client->sendRequest(req);
  if (response.first != drogon::ReqResult::Ok) {
    return false;
  }

  const auto &httpResponse = response.second;
  if (!httpResponse || httpResponse->getStatusCode() != drogon::k201Created) {
    return false;
  }

  const auto &jsonResponse = response.second->getJsonObject();
  if (!jsonResponse) {
    return false;
  }

  const auto idValue = jsonResponse->find(myapp::key::userId.data(),
                                          myapp::key::userId.data() + myapp::key::userId.length());
  if (!idValue || !idValue->isInt64()) {
    return false;
  }
  userMainInfo.id = idValue->asUInt64();
  return true;
}

bool DeleteUser(const UserMainInfo &userMainInfo) {
  auto client = MakeHttpClient();

  auto req = drogon::HttpRequest::newHttpRequest();
  req->setMethod(drogon::Delete);
  req->setPath("/api/v1/user/" + std::to_string(userMainInfo.id));
  req->addHeader("Authorization", myapp::basic::MakeAuth({userMainInfo.username, userMainInfo.password}));

  auto response = client->sendRequest(req);

  if (response.first != drogon::ReqResult::Ok) {
    return false;
  }

  if (!response.second) {
    return false;
  }

  if (response.second->getStatusCode() != drogon::k204NoContent) {
    return false;
  }

  return true;
}

bool AddParamDelimiter(std::ostream &os, bool hasPrevious) {
  if (hasPrevious) {
    os << "&";
  } else {
    os << "?";
  }
  return true;
}

UserListInfo GetList(std::optional<uint64_t> limit, std::optional<uint64_t> offset) {
  auto client = MakeHttpClient();

  auto req = drogon::HttpRequest::newHttpRequest();
  req->setMethod(drogon::Get);
  std::ostringstream os;
  os << "/api/v1/user";
  bool hasQueryParam = false;
  if (limit.has_value()) {
    hasQueryParam = AddParamDelimiter(os, hasQueryParam);
    os << "limit=" << limit.value();
  }
  if (offset.has_value()) {
    hasQueryParam = AddParamDelimiter(os, hasQueryParam);
    os << "offset=" << offset.value();
  }
  req->setPath(os.str());

  const auto response = client->sendRequest(req);
  if (response.first != drogon::ReqResult::Ok) {
    return {};
  }

  const auto &httpResponse = response.second;
  if (!httpResponse || httpResponse->getStatusCode() != drogon::k200OK) {
    return {};
  }

  const auto &jsonResponse = response.second->getJsonObject();
  if (!jsonResponse) {
    return {};
  }

  UserListInfo uli;

  if(!GetValueFromJson(*jsonResponse,myapp::key::listSize,uli.size)){
    return {};
  }

  if(!GetValueFromJson(*jsonResponse,myapp::key::listTotal,uli.total)){
    return {};
  }

  if(!GetOptValueFromJson(*jsonResponse,myapp::key::listLimit,uli.limit)){
    return {};
  }

  if(!GetOptValueFromJson(*jsonResponse,myapp::key::listOffset,uli.offset)){
    return {};
  }

  const auto *usersValue = myapp::FindKey(myapp::key::listUsers, *jsonResponse);
  if (!usersValue || !usersValue->isArray()) {
    return {};
  }

  uli.users.reserve(usersValue->size());
  for (const auto &user : *usersValue) {
    UserMainInfo umi;


    if(!GetValueFromJson(user,myapp::key::userId,umi.id)){
      return {};
    }

    if(!GetValueFromJson(user,myapp::key::username,umi.username)){
      return {};
    }

    uli.users.push_back(std::move(umi));
  }

  return uli;
}

}