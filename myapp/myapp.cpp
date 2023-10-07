#include <user/user_db.h>
#include <myapp/api/v1/user.h>
#include <myapp/program_args.h>

#include <drogon/drogon.h>

int main(int argc, char *argv[]) {

  auto options = myapp::ProgramArgs("myapp", "Simple http server");

  if (!options.Parse(argc, argv, std::cout)) {
    return EXIT_FAILURE;
  }

  auto userDb = std::make_shared<myapp::UserDb>();
  auto userController = std::make_shared<api::v1::User>(userDb);

  drogon::app()
      .loadConfigFile(options.GetServerConfig())
      .registerController(userController)
      .run();

  return EXIT_SUCCESS;
}