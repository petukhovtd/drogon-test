#include <user/user_db.h>
#include <myapp/api/v1/user.h>
#include <myapp/program_args.h>

#include <drogon/drogon.h>

int main(int argc, char *argv[]) {

  try {
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
  }
  catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...) {
    std::cout << "unknown exception" << std::endl;
    throw;
  }

  return EXIT_SUCCESS;
}