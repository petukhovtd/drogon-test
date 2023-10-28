#include <gtest/gtest.h>

#include <libapi/api_keys.h>
#include <libapi/checkers.h>

#include "test_helpers.h"

class UserListGenerator : public testing::Test {

public:
  void Generate(uint64_t count) {
    for (uint64_t i = 0; i < count; ++i) {
      auto umi = myapp::UserMainInfo{"username" + std::to_string(i), "password" + std::to_string(i)};
      ASSERT_TRUE(myapp::CreateUser(umi)) << "add user error";
      users.push_back(umi);
    }
  }

  const myapp::UserMainInfo &Find(uint64_t id) {
    const auto it = std::find_if(users.begin(), users.end(),
                                 [id](const myapp::UserMainInfo &umi) -> bool {
                                   return umi.id == id;
                                 });
    EXPECT_TRUE(it != users.end());
    return *it;
  }

protected:
  virtual void TearDown() {
    for (const auto &umi : users) {
      ASSERT_TRUE(myapp::DeleteUser(umi));
    }
  }

private:
  std::vector<myapp::UserMainInfo> users;
};

TEST_F(UserListGenerator, GetUserListBase) {
  const uint64_t count = 5;
  Generate(count);

  auto client = myapp::MakeHttpClient();

  auto req = drogon::HttpRequest::newHttpRequest();
  req->setMethod(drogon::Get);
  req->setPath("/api/v1/user");

  auto response = client->sendRequest(req);
  EXPECT_TRUE(response.first == drogon::ReqResult::Ok);

  const auto httpResponse = response.second;
  EXPECT_TRUE(httpResponse);
  EXPECT_TRUE(httpResponse->getStatusCode() == drogon::k200OK);

  const auto jsonResponse = response.second->getJsonObject();
  EXPECT_TRUE(jsonResponse);

  const auto *sizeValue = myapp::FindKey(myapp::key::listSize, *jsonResponse);
  EXPECT_TRUE(sizeValue);
  EXPECT_TRUE(sizeValue->isInt64());
  EXPECT_EQ(sizeValue->asUInt64(), count);

  const auto *totalValue = myapp::FindKey(myapp::key::listTotal, *jsonResponse);
  EXPECT_TRUE(totalValue);
  EXPECT_TRUE(totalValue->isInt64());
  EXPECT_EQ(totalValue->asUInt64(), count);

  const auto *usersValue = myapp::FindKey(myapp::key::listUsers, *jsonResponse);
  EXPECT_TRUE(usersValue);
  EXPECT_TRUE(usersValue->isArray());
  EXPECT_EQ(usersValue->size(), count);

  const uint64_t checkId = count / 2;

  const Json::Value &userJson = usersValue[0u][static_cast<Json::ArrayIndex>(checkId)];
  ASSERT_TRUE(userJson.isObject());

  const auto *idValue = myapp::FindKey(myapp::key::userId, userJson);
  EXPECT_TRUE(idValue);
  EXPECT_TRUE(idValue->isInt64());
  const uint64_t id = idValue->asUInt64();

  const auto *usernameValue = myapp::FindKey(myapp::key::username, userJson);
  EXPECT_TRUE(usernameValue);
  EXPECT_TRUE(usernameValue->isString());
  const std::string &username = usernameValue->asString();

  const auto &umi = Find(id);

  EXPECT_STREQ(umi.username.c_str(), username.c_str());
}

struct UserListParam {
  size_t total;
  size_t expectSize;
  std::optional<size_t> offset;
  std::optional<size_t> limit;
};

class GetListParam : public UserListGenerator, public testing::WithParamInterface<UserListParam> {};

TEST_P(GetListParam, Test) {
  const auto &param = GetParam();
  Generate(param.total);

  const auto result = myapp::GetList(param.limit, param.offset);

  EXPECT_EQ(param.total, result.total);
  EXPECT_EQ(param.expectSize, result.size);
}

INSTANTIATE_TEST_SUITE_P(ParamTest, GetListParam, testing::Values(
    UserListParam{10, 10, std::nullopt, std::nullopt},
    UserListParam{10, 10, 0, std::nullopt},
    UserListParam{10, 4, 6, std::nullopt},
    UserListParam{10, 9, 1, std::nullopt},
    UserListParam{10, 1, 9, std::nullopt},
    UserListParam{10, 0, 10, std::nullopt},
    UserListParam{10, 0, 15, std::nullopt},
    UserListParam{10, 0, std::nullopt, 0},
    UserListParam{10, 1, std::nullopt, 1},
    UserListParam{10, 6, std::nullopt, 6},
    UserListParam{10, 10, std::nullopt, 54},
    UserListParam{10, 4, 6, 10},
    UserListParam{10, 5, 3, 5},
    UserListParam{10, 1, 8, 1}
));
