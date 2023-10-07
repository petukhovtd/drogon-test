#include <user/user_db.h>
#include <myapp/api/v1/user.h>

#include <drogon/drogon.h>

#include <cpp-base64/base64.h>

int main() {
  auto userDb = std::make_shared<myapp::UserDb>();
  auto userController = std::make_shared<api::v1::User>(userDb);

  drogon::app()
      .addListener("0.0.0.0", 3000)
      .setThreadNum(1)
      .registerController(userController)
      .run();

  return EXIT_SUCCESS;
}