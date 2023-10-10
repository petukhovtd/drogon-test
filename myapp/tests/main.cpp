#include <gtest/gtest.h>
#include <drogon/drogon.h>
#include <drogon/HttpClient.h>

#include <libapi/api_keys.h>

using namespace drogon;

bool DeleteUser(const std::string& id)
{
  auto client = HttpClient::newHttpClient("http://localhost:3000");
  auto req = HttpRequest::newHttpRequest();
  req->setMethod(Delete);
  req->setPath("/api/v1/user/"+id);
  auto response = client->sendRequest(req);

  bool result = response.first == ReqResult::Ok;
  result &= response.second && response.second->getStatusCode() == k204NoContent;
  return result;
}

TEST(Test,CreateUserSuccsess) {
  auto client = HttpClient::newHttpClient("http://localhost:3000");

  Json::Value body;
  body[myapp::key::username] = "username1";
  body[myapp::key::password] = "password1";

  auto req = HttpRequest::newHttpJsonRequest(body);
  req->setMethod(Post);
  req->setPath("/api/v1/user/create");

  auto response = client->sendRequest(req);
  EXPECT_TRUE(response.first == ReqResult::Ok);

  const auto httpResponse = response.second;
  EXPECT_TRUE(httpResponse);

  EXPECT_TRUE(httpResponse->getStatusCode() == k201Created);

  const auto jsonResponse = response.second->getJsonObject();
  EXPECT_TRUE(jsonResponse);

  const auto idValue = jsonResponse->find(myapp::key::userId.data(), myapp::key::userId.data() + myapp::key::userId.length());
  EXPECT_TRUE(idValue);
  EXPECT_TRUE(idValue->isInt64());
  auto id = idValue->as<Json::UInt64>();

  DeleteUser(std::to_string(id));
}

TEST(Test, CreateUserError) {
  auto client = HttpClient::newHttpClient("http://localhost:3000");
  // Empty body
  {
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Post);
    req->setPath("/api/v1/user/create");

    auto response = client->sendRequest(req);
    EXPECT_TRUE(response.first == ReqResult::Ok);

    const auto httpResponse = response.second;
    EXPECT_TRUE(httpResponse);

    EXPECT_TRUE(httpResponse->getStatusCode() == k400BadRequest);

    const auto jsonResponse = response.second->getJsonObject();
    EXPECT_TRUE(jsonResponse);
    std::cout << response.second->body() << '\n';
  }
}

int main(int argc, char **argv) {

  std::system("pwd");

  std::promise<void> p1;
  std::future<void> f1 = p1.get_future();

  // Start the main loop on another thread
  std::thread thr([&]() {
    // Queues the promise to be fulfilled after starting the loop
    app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
    app().run();
  });

  // The future is only satisfied after the event loop started
  f1.get();

  testing::InitGoogleTest(&argc, argv);
  int status = RUN_ALL_TESTS();

  // Ask the event loop to shutdown and wait
  app().getLoop()->queueInLoop([]() { app().quit(); });
  thr.join();
  return status;
}
