#pragma once

#include <cxxopts.hpp>

#include <ostream>

namespace myapp {

class ProgramArgs {

public:

  ProgramArgs(const std::string &appName, const std::string &description);

  bool Parse(int argc, char *argv[], std::ostream &os);

  const std::string &GetServerConfig() const;

private:
  cxxopts::Options options_;
  std::string serverConfig_;
};

}
