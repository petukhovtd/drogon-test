#include <user/user_db.h>
#include <api/v1/user.h>

namespace myapp {

drogon::HttpAppFramework &RegisterUserApi(drogon::HttpAppFramework &app) {

  auto userDb = std::make_shared<myapp::UserDb>();
  auto userController = std::make_shared<api::v1::User>(userDb);

  app.registerController(userController);

  return app;
}

}
