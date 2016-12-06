#ifndef DECODER_HPP
#define DECODER_HPP
#include <algorithm>
#include <cassert>
#include <future>
#include <numeric>
#include <vector>

namespace lzw {
template <class Value> class decoder {
  // wanted to use std::vector<char>, but this structure doesn't has the hash
  // function in unordered map :(
  using map_type = std::vector<Value>;

private:
  map_type _map;

public:
  decoder() {
    for (auto i = 0; i < 0xff; i++) {
      _map.emplace_back(1, i);
    }
    assert(_map.size() == 0xff);
  }

  decoder(const decoder &object) = default;
  decoder(decoder &&object) = default;
  decoder &operator=(const decoder &object) = default;
  decoder &operator=(decoder &&object) = default;
  ~decoder() = default;

  template <class InputIter>
  std::vector<char> decode(InputIter begin, InputIter end) {
    uint16_t current_code = *begin;
    uint16_t previous_code = current_code;
    Value previous;
    Value current;
    assert(current_code < _map.size());
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
        _map.emplace_back(previous + current);
      } else {
        assert(current_code == _map.size());
        assert(previous_code < _map.size());
        previous = _map[previous_code];
        current = std::string(1, _map[previous_code][0]);
        decoded.insert(decoded.end(), previous.begin(), previous.end());
        decoded.emplace_back(current[0]);
      }
      previous_code = current_code;
      ++begin;
    }
    return decoded;
  }

  //���� �� ����� �� ������� ������������������ \n, ������ ����, ��� ������ ����
  //�������� � ��� �� �����
  template <class RandomAccessIter>
  static std::vector<char> parallel_decode(RandomAccessIter begin,
                                           RandomAccessIter end) {
    std::vector<std::future<std::vector<char>>> future_vector;
    auto first = begin;
    auto last = begin;
    while (last != end) {
      do {
        last = std::find(last, end, '\n');
      } while (last != end && *(last - 1) == '\\');
      future_vector.push_back(std::async([first, last]() {
        decoder thread_decoder;
        std::vector<char> result = thread_decoder.decode(first, last);
        return result;
      }));
      if (last != end) {
        first = last + 1;
      }
    }
    for (auto &f : future_vector) {
      f.wait();
    }
    if (!std::all_of(future_vector.begin(), future_vector.end(),
                     [](auto &future) { return future.valid(); })) {
      throw std::runtime_error("GOSPODIN, VI GOOS'");
    }
    std::vector<char> result;
    for (auto &future : future_vector) {
      const auto &part = future.get();
      std::copy(part.begin(), part.end(), std::back_inserter(result));
    }
    return result;
  }
};
}
#endif // DECODER_HPP
