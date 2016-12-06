#ifndef ENCODER_HPP
#define ENCODER_HPP
#include <algorithm>
#include <future>
#include <thread>
#include <unordered_map>
#include <vector>

namespace lzw {
template <class Key> class encoder {
  // wanted to use std::vector<char>, but this structure doesn't has the hash
  // function in unordered map :(
  using map_type = std::unordered_map<Key, uint16_t>;
  using data_type = std::vector<uint16_t>;

  map_type _map;

public:
  using parallel_encoder_type = std::vector<data_type>;
  encoder() {
    for (int i = 0; i < 0xff; i++) {
      _map.emplace(std::string(1, i), i);
    }
  }

  encoder(const encoder &object) = default;
  encoder(encoder &&object) = default;
  encoder &operator=(const encoder &object) = default;
  encoder &operator=(encoder &&object) = default;
  ~encoder() = default;

  template <class InputIter> data_type encode(InputIter begin, InputIter end) {
    // init block
    Key previous;      // previous character/characters
    Key current;       // current character/characters
    data_type encoded; // main output -- encoded string

    // main cycle
    while (begin != end) {
      current = *begin;

      if (_map.find(previous + current) !=
          _map.end()) { // if we find pair of character in hashtable
        previous += current;
      } else { // if we don't find
        auto search = _map.find(previous);
        encoded.emplace_back(search->second);
        _map.emplace(previous + current, _map.size());
        previous = current;
      }
      ++begin;
    }
    auto search = _map.find(current);
    encoded.emplace_back(search->second);
    return encoded;
  }

  template <class RandomAccessIter>
  static std::vector<data_type> parallel_encode(RandomAccessIter begin,
                                                RandomAccessIter end) {
    const unsigned int size = end - begin;
    const unsigned int nthreads = std::thread::hardware_concurrency();
    const unsigned int size_per_thread = size / nthreads;
    std::vector<std::future<data_type>> future_vector;
    for (auto i = 0; i < nthreads - 1; i++) {
      future_vector.push_back(std::async([begin, i, size_per_thread]() {
        encoder thread_encoder;
        auto start_iter = begin + i * size_per_thread;
        data_type result =
            thread_encoder.encode(start_iter, start_iter + size_per_thread);
        return result;
      }));
    }

    // {TRICKY TRICK} if size % nthread != 0 we don't give a shit, cause we did
    // that staff below

    future_vector.push_back(
        std::async([begin, end, nthreads, size_per_thread]() {
          encoder thread_encoder;
          auto start_iter = begin + (nthreads - 1) * size_per_thread;
          data_type result = thread_encoder.encode(start_iter, end);
          return result;
        }));

    for (auto &f : future_vector) {
      f.wait();
    }
    if (!std::all_of(future_vector.begin(), future_vector.end(),
                     [](auto &future) { return future.valid(); })) {
      throw std::runtime_error("GOSPODIN, VI GOOS'");
    }
    std::vector<data_type> results(nthreads);
    std::transform(future_vector.begin(), future_vector.end(), results.begin(),
                   [](auto &future) { return future.get(); });
    return results;
  }
};

// dictionary init_map(const char* input, std::size_t count);

//	std::pair<dictionary, std::vector<uint16_t>> encode(const char* input,
// size_t input_size);
}
#endif // ENCODER_HPP
