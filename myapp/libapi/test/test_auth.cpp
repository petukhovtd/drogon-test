#include <gtest/gtest.h>

#include <libapi/auth.h>

TEST(Auth, SplitBy) {
  {
    const auto res = myapp::SplitBy("123:456", ":");
    EXPECT_TRUE(res.first == "123");
    EXPECT_TRUE(res.second == "456");
  }
  {
    const auto res = myapp::SplitBy("123::456", "::");
    EXPECT_TRUE(res.first == "123");
    EXPECT_TRUE(res.second == "456");
  }
  {
    const auto res = myapp::SplitBy(":456", ":");
    EXPECT_TRUE(res.first == "");
    EXPECT_TRUE(res.second == "456");
  }
  {
    const auto res = myapp::SplitBy("123:", ":");
    EXPECT_TRUE(res.first == "123");
    EXPECT_TRUE(res.second == "");
  }
  {
    const auto res = myapp::SplitBy("123456", ":");
    EXPECT_TRUE(res.first == "123456");
    EXPECT_TRUE(res.second == "");
  }
}

TEST(Auth, GetCredentials) {
  {
    const auto result = myapp::basic::GetCredentials("Basic bm9uZTpub25l");
    EXPECT_TRUE(std::holds_alternative<myapp::basic::Credentials>(result));
    const auto credentials = std::get<myapp::basic::Credentials>(result);
    EXPECT_STREQ(credentials.username.c_str(), "none");
    EXPECT_STREQ(credentials.password.c_str(), "none");
  }
  {
    const auto result = myapp::basic::GetCredentials("Basic dXNlcm5hbWU6cGFzc3dvcmQ=");
    EXPECT_TRUE(std::holds_alternative<myapp::basic::Credentials>(result));
    const auto credentials = std::get<myapp::basic::Credentials>(result);
    EXPECT_STREQ(credentials.username.c_str(), "username");
    EXPECT_STREQ(credentials.password.c_str(), "password");
  }
  {
    const auto result = myapp::basic::GetCredentials("Basic Og==");
    EXPECT_TRUE(std::holds_alternative<myapp::basic::Credentials>(result));
    const auto credentials = std::get<myapp::basic::Credentials>(result);
    EXPECT_STREQ(credentials.username.c_str(), "");
    EXPECT_STREQ(credentials.password.c_str(), "");
  }
  {
    const auto result = myapp::basic::GetCredentials("bm9uZTpub25l");
    EXPECT_TRUE(std::holds_alternative<myapp::Error>(result));
    EXPECT_EQ(std::get<myapp::Error>(result).GetCode(), myapp::Error::Code::AuthorizateFailed);
  }
  {
    const auto result = myapp::basic::GetCredentials("");
    EXPECT_TRUE(std::holds_alternative<myapp::Error>(result));
    EXPECT_EQ(std::get<myapp::Error>(result).GetCode(), myapp::Error::Code::AuthorizateFailed);
  }
}

TEST(Auth, MakeAuth) {
    EXPECT_STREQ(myapp::basic::MakeAuth({"none", "none"}).c_str(), "Basic bm9uZTpub25l");
    EXPECT_STREQ(myapp::basic::MakeAuth({"username", "password"}).c_str(), "Basic dXNlcm5hbWU6cGFzc3dvcmQ=");
    EXPECT_STREQ(myapp::basic::MakeAuth({"ajfhgoaifg", "bpoaidgb"}).c_str(), "Basic YWpmaGdvYWlmZzpicG9haWRnYg==");
    EXPECT_STREQ(myapp::basic::MakeAuth({"", ""}).c_str(), "Basic Og==");
}
