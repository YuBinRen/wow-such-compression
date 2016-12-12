#include "encoder.hpp"

namespace lzw {

encoder::encoder() {
  for (int i = 0; i < 0xff; i++) {
    _map.emplace(std::string(1, static_cast<char>(i)), i);
  }
}
}
