#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <iterator>
#include <type_traits>

namespace lzw {

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

  // TODO: move this logic to operator++
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

  bool operator!=(const UnescapeIterator &rhs) const {
    return _inner != rhs._inner;
  }
  bool operator==(const UnescapeIterator &rhs) const { return !(*this != rhs); }
};

template <class Iter> UnescapeIterator<Iter> unescape(Iter &&it) {
  return UnescapeIterator<Iter>(std::forward<Iter>(it));
}
} // namespace lzw

#endif // UTILITIES_HPP
