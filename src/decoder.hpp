#ifndef DECODER_HPP
#define DECODER_HPP

#include "utilities.hpp"

#include <algorithm>
#include <cassert>
#include <future>
#include <numeric>
#include <vector>

namespace lzw {

class decoder {
  // wanted to use std::vector<char>, but this structure doesn't has the hash
  // function in unordered map :(
  using map_t = std::vector<std::string>;

private:
  map_t _map;

public:
  decoder();
  decoder(const decoder &object) = default;
  decoder(decoder &&object) = default;
  decoder &operator=(const decoder &object) = default;
  decoder &operator=(decoder &&object) = default;
  ~decoder() = default;

  // TODO: refactor this
  template <class InputIter>
  std::vector<char> decode(InputIter begin, InputIter end) {
    if (begin == end) {
      return std::vector<char>();
    }
    uint16_t current_code = *begin;
    uint16_t previous_code = current_code;
    std::string previous;
    std::string current;
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
        //#TODO: need to test
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
                                           const uint16_t *end);
};
}
#endif // DECODER_HPP
