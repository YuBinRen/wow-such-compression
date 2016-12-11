#ifndef DECODER_HPP
#define DECODER_HPP

#include <algorithm>
#include <cassert>
#include <future>
#include <iterator>
#include <numeric>
#include <vector>

// for debug
#include <iostream>

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

template <class Iter>
class UnescapeIterator
    : public std::iterator<std::forward_iterator_tag,
                           typename std::remove_reference<decltype(
                               *std::declval<Iter>())>::type> {
  Iter _inner;

  // state, that show us, do we did a skip at previous dereference
  bool _skip_state = false;

public:
  UnescapeIterator(Iter it) : _inner(it) {}
  UnescapeIterator(const UnescapeIterator &) = default;
  UnescapeIterator(UnescapeIterator &&) = default;
  UnescapeIterator &operator=(const UnescapeIterator &) = default;
  UnescapeIterator &operator=(UnescapeIterator &&) = default;
  ~UnescapeIterator() = default;

  UnescapeIterator operator++() {
    ++_inner;
    return *this;
  }

  typename std::remove_reference<decltype(*std::declval<Iter>())>::type
  operator*() {
    if (_skip_state == false && *_inner == '\\') {
      ++_inner;
      _skip_state = true;
    } else {
      _skip_state = false;
    }
    return *_inner;
  }

  bool operator!=(const UnescapeIterator &rhs) { return _inner != rhs._inner; }
  bool operator==(const UnescapeIterator &rhs) { return !(*this != rhs); }
};

template <class Iter> UnescapeIterator<Iter> unescape(Iter &&it) {
  return UnescapeIterator<Iter>(std::forward<Iter>(it));
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

  template <class RandomAccessIter>
  static std::vector<char> parallel_decode(const RandomAccessIter begin,
                                           const RandomAccessIter end) {
    using future_t = std::future<std::vector<char>>;
    std::vector<future_t> futures;

    auto current = begin;
    auto last = find_unescaped(current, end, '\n');
    while (last != end) {
      futures.emplace_back(
          std::async([ from = unescape(current), to = unescape(last) ]() {
            decoder local_decoder;
            return local_decoder.decode(from, to);
          }));
      current = last + 1;
      last = find_unescaped(current, end, '\n');
    }

    std::vector<char> decoded;
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
