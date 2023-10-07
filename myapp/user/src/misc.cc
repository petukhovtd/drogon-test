#include <user/misc.h>

#include <openssl/md5.h>

#include <sstream>
#include <iomanip>

namespace myapp {

std::string CalculateMd5(const std::string &s) {
  unsigned char md5digest[MD5_DIGEST_LENGTH];

  MD5(reinterpret_cast<const unsigned char *>(s.c_str()), s.length(), md5digest);

  std::stringstream ss;
  ss << std::setw(2) << std::setfill('0') << std::setbase(16);
  for (auto c : md5digest) {
    ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned int>(c);
  }

  return ss.str();
}

}