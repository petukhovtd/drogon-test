#include <user/user_db.h>
#include <api/v1/user_http_controller.h>

namespace myapp {

drogon::HttpAppFramework &RegisterUserApi(drogon::HttpAppFramework &app) {

  auto userDb = std::make_shared<myapp::UserDb>();
  auto userController = std::make_shared<myapp::UserHttpController>(userDb);

  app.registerController(userController);

  return app;
}

}
