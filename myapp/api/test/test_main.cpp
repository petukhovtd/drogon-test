#include <gtest/gtest.h>
#include <drogon/drogon.h>
#include <api/api_user.h>

#include "test_helpers.h"

int main(int argc, char **argv) {

  std::promise<void> startFlag;
  std::future<void> waitStartFlag = startFlag.get_future();

  std::thread thr([&]() {
    myapp::RegisterUserApi(drogon::app());
    const auto serverAddress = myapp::GetServerAddress();
    drogon::app().addListener(serverAddress.ip, serverAddress.port);
    drogon::app().getLoop()->queueInLoop([&startFlag]() { startFlag.set_value(); });
    drogon::app().run();
  });

  waitStartFlag.get();

  testing::InitGoogleTest(&argc, argv);
  int status = RUN_ALL_TESTS();

  drogon::app().getLoop()->queueInLoop([]() { drogon::app().quit(); });
  thr.join();

  return status;
}
