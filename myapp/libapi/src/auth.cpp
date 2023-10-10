#include <libapi/auth.h>

#include <cpp-base64/base64.h>

namespace myapp {

std::pair<std::string_view, std::string_view> SplitBy(std::string_view s, std::string_view delimiter) {
  const size_t pos = s.find(delimiter);
  if (s.npos == pos) {
    return {s, std::string_view()};
  }
  std::string_view lhs = s.substr(0, pos);
  std::string_view rhs = s.substr(pos + delimiter.size());
  return {lhs, rhs};
}

namespace basic {

std::variant<Credentials, Error> GetCredentials(std::string_view authHeader) {
  const auto credentialsBase = SplitBy(authHeader, " ");

  if (credentialsBase.first != "Basic") {
    return Error(myapp::Error::Code::AuthorizateFailed, {{"why", "authorization type not Basic"}});
  }

  std::string credentialsDecode = base64_decode(credentialsBase.second);

  const auto credentials = SplitBy(credentialsDecode, ":");

  return Credentials{std::string(credentials.first), std::string(credentials.second)};
}

std::string MakeAuth(const Credentials &credentials) {
  std::ostringstream os;
  os << credentials.username << ":" << credentials.password;
  const auto credentialsEncode = base64_encode(os.str());
  os.str("");
  os << "Basic " << credentialsEncode;
  return os.str();
}

}
}