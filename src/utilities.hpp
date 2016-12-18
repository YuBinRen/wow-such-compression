#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <algorithm>
#include <iostream>
#include <iterator>
#include <type_traits>

namespace lzw {

template <class Iter>
class unescape_iterator
    : public std::iterator<std::forward_iterator_tag,
                           typename std::remove_reference<decltype(
                               *std::declval<Iter>())>::type> {
  Iter _inner;

public:
  unescape_iterator(Iter it) : _inner(it) {}
  unescape_iterator() = default;
  unescape_iterator(const unescape_iterator &) = default;
  unescape_iterator(unescape_iterator &&) = default;
  unescape_iterator &operator=(const unescape_iterator &) = default;
  unescape_iterator &operator=(unescape_iterator &&) = default;
  ~unescape_iterator() = default;

  unescape_iterator operator++() {
    ++_inner;
    if (*_inner == '\\') {
      ++_inner;
    }
    return *this;
  }

  typename std::remove_reference<decltype(*std::declval<Iter>())>::type
  operator*() {
    return *_inner;
  }

  bool operator!=(const unescape_iterator &rhs) const {
    return _inner != rhs._inner;
  }
  bool operator==(const unescape_iterator &rhs) const {
    return _inner == rhs._inner;
  }

  std::ptrdiff_t operator-(const unescape_iterator &rhs) const {
    return _inner - rhs._inner;
  }
};

template <class Iter> unescape_iterator<Iter> unescape(Iter &&it) {
  return unescape_iterator<Iter>(std::forward<Iter>(it));
}

const uint16_t *find_unescaped(const uint16_t *begin, const uint16_t *end,
                               const uint16_t &what_to_find);

} // namespace lzw

#endif // UTILITIES_HPP
