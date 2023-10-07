#include <myapp/program_args.h>

namespace myapp {

ProgramArgs::ProgramArgs(const std::string &appName, const std::string &description)
    : options_("myapp", "Simple http server"), serverConfig_() {
  options_.add_options()
      ("s,server", "Server config file (drogon)", cxxopts::value<std::string>())
      ("h,help", "Print usage");
}

bool ProgramArgs::Parse(int argc, char **argv, std::ostream &os) {
  auto result = options_.parse(argc, argv);

  if (result.count("help")) {
    os << options_.help() << '\n';
    return false;
  }

  bool noError = true;

  if (result.count("server")) {
    serverConfig_ = result["server"].as<std::string>();
  } else {
    os << "error: server config is required" << '\n';
    noError = false;
  }

  return noError;
}

const std::string &ProgramArgs::GetServerConfig() const {
  return serverConfig_;
}

}
