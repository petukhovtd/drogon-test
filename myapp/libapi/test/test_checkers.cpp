#include <gtest/gtest.h>

#include <jsoncpp/json/json.h>

#include <libapi/checkers.h>

TEST(CheckersTest, GetStringFoundKey) {
  constexpr const auto key = "testkey";
  constexpr const auto value = "testvalue";

  Json::Value json;
  json[key] = value;

  const auto getResult = myapp::GetString(key, json);
  EXPECT_TRUE(std::holds_alternative<std::string>(getResult));
  EXPECT_STREQ(std::get<std::string>(getResult).c_str(), value);
}

TEST(CheckersTest, GetStringNotFoundKey) {
  constexpr const auto key = "testkey";

  Json::Value json;

  const auto getResult = myapp::GetString(key, json);
  EXPECT_TRUE(std::holds_alternative<myapp::Error>(getResult));
  EXPECT_EQ(std::get<myapp::Error>(getResult).GetCode(), myapp::Error::Code::KeyNotFound);
}

TEST(CheckersTest, GetStringInvalidType) {
  constexpr const auto key = "testkey";

  Json::Value json;
  json[key] = 1;

  const auto getResult = myapp::GetString(key, json);
  EXPECT_TRUE(std::holds_alternative<myapp::Error>(getResult));
  EXPECT_EQ(std::get<myapp::Error>(getResult).GetCode(), myapp::Error::Code::InvalidType);
}

TEST(CheckersTest, CheckUsernameSuccess) {
  EXPECT_FALSE(myapp::CheckUsername("12345").has_value());
  EXPECT_FALSE(myapp::CheckUsername("username").has_value());
  EXPECT_FALSE(myapp::CheckUsername("user.name").has_value());
  EXPECT_FALSE(myapp::CheckUsername("username123").has_value());
  EXPECT_FALSE(myapp::CheckUsername("use123rna.me").has_value());
  EXPECT_FALSE(myapp::CheckUsername("321usAAERe123rna.me").has_value());
  EXPECT_FALSE(myapp::CheckUsername(std::string(128,'a')).has_value());
}

TEST(CheckersTest, CheckUsernameInvalidSize) {
  EXPECT_TRUE(myapp::CheckUsername("").has_value());
  EXPECT_TRUE(myapp::CheckUsername("1234").has_value());
  EXPECT_TRUE(myapp::CheckUsername(std::string(129,'a')).has_value());
  EXPECT_TRUE(myapp::CheckUsername(std::string(256,'a')).has_value());
}

TEST(CheckersTest, CheckUsernameInvalidChar) {
  EXPECT_TRUE(myapp::CheckUsername(".username").has_value());
  EXPECT_TRUE(myapp::CheckUsername("user-name").has_value());
  EXPECT_TRUE(myapp::CheckUsername("username-").has_value());
  EXPECT_TRUE(myapp::CheckUsername("user_name").has_value());
  EXPECT_TRUE(myapp::CheckUsername("username_").has_value());
  EXPECT_TRUE(myapp::CheckUsername("userna!me").has_value());
}