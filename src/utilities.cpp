#include "utilities.hpp"

namespace lzw {

const uint16_t *find_unescaped(const uint16_t *begin, const uint16_t *end,
                               const uint16_t &what_to_find) {
  auto current = std::find(begin, end, what_to_find);
  if (current == begin) {
    return current;
  }
  while (current != end && *(current - 1) == '\\') {
    ++current;
    current = std::find(current, end, what_to_find);
  }
  return current;
}
}
