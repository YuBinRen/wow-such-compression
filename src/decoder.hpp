#ifndef DECODER_HPP
#define DECODER_HPP

#include "utilities.hpp"

#include <algorithm>
#include <cassert>
#include <future>
#include <numeric>
#include <vector>

namespace lzw {

// TODO: test it
template <class RandomAccessIter, class T>
RandomAccessIter find_unescaped(const RandomAccessIter begin,
                                const RandomAccessIter end,
                                const T &what_to_find) {
  auto current = std::find(begin, end, what_to_find);
  if (current == begin) {
    return current;
  }

  while (current != end && *(current - 1) == '\\') {
    current = std::find(current, end, what_to_find);
  }
  return current;
}

template <class Value> class decoder {
  // wanted to use std::vector<char>, but this structure doesn't has the hash
  // function in unordered map :(
  using map_t = std::vector<Value>;

private:
  map_t _map;

public:
  decoder() {
    for (auto i = 0; i <= 0xff; i++) {
      _map.emplace_back(1, i);
    }
  }

  decoder(const decoder &object) = default;
  decoder(decoder &&object) = default;
  decoder &operator=(const decoder &object) = default;
  decoder &operator=(decoder &&object) = default;
  ~decoder() = default;

  // TODO: refactor this
  template <class InputIter>
  std::vector<char> decode(InputIter begin, InputIter end) {
    uint16_t current_code = *begin;
    uint16_t previous_code = current_code;
    Value previous;
    Value current;
    assert(current_code < _map.size());
    // first symbol.size() is always 1byte
    std::vector<char> decoded = {
        _map[current_code][0]}; // main output -- decoded file

    ++begin;
    while (begin != end) {
      current_code = *begin;
      if (current_code < _map.size()) {
        const auto &temp = _map[current_code];
        decoded.insert(decoded.end(), temp.begin(), temp.end());
        assert(previous_code < _map.size());
        previous = _map[previous_code];
        current = std::string(1, temp[0]);
      } else {
        assert(current_code == _map.size());
        assert(previous_code < _map.size());
        previous = _map[previous_code];
        current = std::string(1, _map[previous_code][0]);

        decoded.insert(decoded.end(), previous.begin(), previous.end());
        decoded.emplace_back(current[0]);
      }
      _map.emplace_back(previous + current);
      previous_code = current_code;
      ++begin;
    }
    return decoded;
  }

  static std::vector<char> parallel_decode(const uint16_t *begin,
                                           const uint16_t *end) {
    using future_t = std::future<std::vector<char>>;
    std::vector<future_t> futures;

    auto current = begin;
    auto last = find_unescaped(current, end, '\n');
    while (last != end) {
      std::cerr << futures.size() << std::endl;
      futures.emplace_back(
          std::async([ from = unescape(current), to = unescape(last) ]() {
            // std::cerr << std::hex << (*from) << std::endl;
            // std::cerr << std::hex << (*to) << std::endl;
            decoder local_decoder;
            return local_decoder.decode(from, to);
          }));
      current = last + 1;
      last = find_unescaped(current, end, '\n');
    }

    std::vector<char> decoded;
    std::cerr << futures.size();
    assert(futures.size() == 1);
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
};
}
#endif // DECODER_HPP
