#include "protector.hpp"

#include <cstdint>
#include <string>

#include <boost/iostreams/device/mapped_file.hpp>

const static std::string path = "guard.urmom";

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return 1;
  }
  int32_t access_value = -1;
  protector guard;
  std::array<uint8_t, 256> buffer;
  *reinterpret_cast<int32_t *>(buffer.data()) = access_value;
  guard.aes_encrypt(buffer.data());
  boost::iostreams::mapped_file file(path);
  std::copy(buffer.data(), buffer.data() + buffer.size(), file.data());

  return 0;
}
