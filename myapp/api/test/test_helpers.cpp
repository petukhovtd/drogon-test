#include "test_helpers.h"

#include <libapi/auth.h>
#include <libapi/api_keys.h>

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
  userMainInfo.id = idValue->as<Json::UInt64>();
  return true;
}

bool DeleteUser(const UserMainInfo &userMainInfo) {
  auto client = MakeHttpClient();

  auto req = drogon::HttpRequest::newHttpRequest();
  req->setMethod(drogon::Delete);
  req->setPath("/api/v1/user/" + std::to_string(userMainInfo.id));
  req->addHeader("Authorization", myapp::basic::MakeAuth({userMainInfo.username, userMainInfo.password}));

  auto response = client->sendRequest(req);

  bool result = response.first == drogon::ReqResult::Ok;
  result &= response.second && response.second->getStatusCode() == drogon::k204NoContent;
  return result;
}

}