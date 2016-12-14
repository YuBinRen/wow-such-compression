#include "decoder.hpp"
#include "utilities.hpp"
#include <iostream>

namespace lzw {
decoder::decoder() {
  for (auto i = 0; i <= 0xff; i++) {
    _map.emplace_back(1, i);
  }
}

// TODO: fix data loseness

std::vector<char> decoder::parallel_decode(const uint16_t *begin,
                                           const uint16_t *end) {
  using future_t = std::future<std::vector<char>>;
  using unescape_iter_t = UnescapeIterator<const uint16_t *>;
  std::vector<future_t> futures;

  const uint16_t *current = begin;
  const uint16_t *last = find_unescaped(current, end, '\n');
  while (last != end) {
    // TODO: check that if doesn't break this function
    if (*current == '\\') {
      ++current;
    }
    auto from = unescape_iter_t(current);
    auto to = unescape_iter_t(last);
    futures.emplace_back(std::async(std::launch::async, [from, to]() {
      decoder local_decoder;
      return local_decoder.decode(from, to);
    }));
    current = last + 1;
    last = find_unescaped(current, end, '\n');
  }

  std::vector<char> decoded;
  for (auto &future : futures) {
    if (future.valid()) {
      const auto &data = future.get();
      decoded.insert(decoded.end(), data.begin(), data.end());
    } else {
      throw std::runtime_error("Something going bad.");
    }
  }
  return decoded;
}
} // namespace lzw
