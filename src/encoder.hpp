#ifndef ENCODER_HPP
#define ENCODER_HPP
#include <algorithm>
#include <cassert>
#include <future>
#include <thread>
#include <unordered_map>
#include <vector>

#include <iostream>

namespace lzw {
class encoder {
  // wanted to use std::vector<char>, but this structure doesn't has the hash
  // function in unordered map :(
  using map_t = std::unordered_map<std::string, uint16_t>;
  using data_t = std::vector<uint16_t>;

  map_t _map;

public:
  using parallel_encoded_data_t = std::vector<data_t>;

  encoder();
  encoder(const encoder &object) = default;
  encoder(encoder &&object) = default;
  encoder &operator=(const encoder &object) = default;
  encoder &operator=(encoder &&object) = default;
  ~encoder() = default;

  template <class InputIter> data_t encode(InputIter begin, InputIter end) {
    // init block
    std::string previous; // previous character/characters
    std::string current;  // current character/characters
    data_t encoded;       // main output -- encoded string

    // main cycle
    while (begin != end) {
      current = *begin;
      if (_map.find(previous + current) !=
          _map.end()) { // if we find pair of character in hashtable
        previous += current;
      } else { // if we don't find
        auto search = _map.find(previous);
        assert(search != _map.end());
        encoded.emplace_back(search->second);
        if (_map.size() < std::numeric_limits<uint16_t>::max()) {
          _map.emplace(previous + current, _map.size());
        }
        previous = current;
      }
      ++begin;
    }

    if ((previous + current).size() == 1) {
      auto search = _map.find(current);
      assert(search != _map.end());
      encoded.emplace_back(search->second);
    } else {
      auto search = _map.find(previous);
      assert(search != _map.end());
      encoded.emplace_back(search->second);
    }

    // auto search = _map.find(current);
    // assert(search != _map.end());
    // encoded.emplace_back(search->second);

    return encoded;
  }

  template <class RandomAccessIter>
  static std::vector<data_t> parallel_encode(const RandomAccessIter begin,
                                             const RandomAccessIter end) {
    using future_t = std::future<data_t>;
    const auto size = end - begin;

    const auto nthreads = size < 1024 ? 1 : std::thread::hardware_concurrency();
    const auto size_per_thread = size / nthreads;
    // const auto nthreads = size / size_per_thread;

    std::vector<future_t> futures;
    for (unsigned int i = 0; i < nthreads - 1; i++) {
      futures.emplace_back(std::async(
          [ start = begin + i * size_per_thread, size_per_thread ]() {
            encoder local_encoder;
            return local_encoder.encode(start, start + size_per_thread);
          }));
    }

    futures.emplace_back(
        std::async([ start = begin + (nthreads - 1) * size_per_thread, end ]() {
          encoder local_encoder;
          return local_encoder.encode(start, end);
        }));

    std::cerr << "Using " << futures.size() << " threads" << std::endl;
    std::vector<data_t> encoded_data;
    for (auto &&future : futures) {
      if (future.valid()) {
        std::cerr << "TYT EMPLACE NIZHE" << std::endl;
        encoded_data.emplace_back(future.get());
      } else {
        throw std::runtime_error("Something going wrong.");
      }
    }
    return encoded_data;
  }
};
}
#endif // ENCODER_HPP
