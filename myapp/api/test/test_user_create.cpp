#include <gtest/gtest.h>

#include <drogon/drogon.h>

#include "test_helpers.h"

#include <libapi/api_keys.h>

TEST(CreateUserTest, CreateUserSuccsess) {
  const std::string username = "username1";
  const std::string password = "password1";

  auto client = myapp::MakeHttpClient();

  Json::Value body;
  body[myapp::key::username] = username;
  body[myapp::key::password] = password;

  auto req = drogon::HttpRequest::newHttpJsonRequest(body);
  req->setMethod(drogon::Post);
  req->setPath("/api/v1/user/create");

  auto response = client->sendRequest(req);
  EXPECT_TRUE(response.first == drogon::ReqResult::Ok);

  const auto httpResponse = response.second;
  EXPECT_TRUE(httpResponse);
  EXPECT_TRUE(httpResponse->getStatusCode() == drogon::k201Created);

  const auto jsonResponse = response.second->getJsonObject();
  EXPECT_TRUE(jsonResponse);

  const auto
      idValue = jsonResponse->find(myapp::key::userId.data(), myapp::key::userId.data() + myapp::key::userId.length());
  EXPECT_TRUE(idValue);
  EXPECT_TRUE(idValue->isInt64());
  auto id = idValue->as<Json::UInt64>();

  EXPECT_TRUE(myapp::DeleteUser({username, password, id}));
}

struct CreateUserParam {
  drogon::HttpStatusCode code;
  bool addJson;
  std::optional<std::string> username;
  std::optional<std::string> password;
  bool createDuplicate;
};

class CreateUserParamFixture : public testing::TestWithParam<CreateUserParam> {};

TEST_P(CreateUserParamFixture, CreateUserError) {
  const auto &param = GetParam();

  if (param.createDuplicate) {
    myapp::UserMainInfo umi;
    umi.username = param.username.value();
    umi.password = param.password.value();
    EXPECT_TRUE(myapp::CreateUser(umi));
  }

  auto client = myapp::MakeHttpClient();

  auto req = [&param]() {
    if (!param.addJson) {
      return drogon::HttpRequest::newHttpRequest();
    }

    Json::Value jsonBody;
    if (param.username.has_value()) {
      jsonBody[myapp::key::username] = param.username->c_str();
    }
    if (param.password.has_value()) {
      jsonBody[myapp::key::password] = param.password->c_str();
    }
    return drogon::HttpRequest::newHttpJsonRequest(jsonBody);
  }();

  req->setMethod(drogon::Post);
  req->setPath("/api/v1/user/create");

  auto response = client->sendRequest(req);
  EXPECT_TRUE(response.first == drogon::ReqResult::Ok);

  const auto httpResponse = response.second;
  EXPECT_TRUE(httpResponse);
  EXPECT_TRUE(httpResponse->getStatusCode() == param.code);

  const auto jsonResponse = response.second->getJsonObject();
  EXPECT_TRUE(jsonResponse);
  std::cout << response.second->body() << '\n';
}

INSTANTIATE_TEST_CASE_P
(CreateUserErrorTest, CreateUserParamFixture, ::testing::Values(
    CreateUserParam{drogon::k400BadRequest, false, std::nullopt, std::nullopt},
    CreateUserParam{drogon::k400BadRequest, true, std::nullopt, std::nullopt},
    CreateUserParam{drogon::k400BadRequest, true, "username", std::nullopt},
    CreateUserParam{drogon::k400BadRequest, true, std::nullopt, "password"},
    CreateUserParam{drogon::k409Conflict, true, "username", "password", true}
));
