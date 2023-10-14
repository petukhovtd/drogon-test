#include <myapp/program_args.h>
#include <api/api_user.h>

#include <drogon/drogon.h>

int main(int argc, char *argv[]) {

  try {
    auto options = myapp::ProgramArgs("myapp", "Simple http server");

    if (!options.Parse(argc, argv, std::cout)) {
      return EXIT_FAILURE;
    }

    auto &app = drogon::app().loadConfigFile(options.GetServerConfig());
    myapp::RegisterUserApi(app).run();

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