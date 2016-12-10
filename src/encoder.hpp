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
		using map_t = std::unordered_map<Key, uint16_t>;
		using data_t = std::vector<uint16_t>;

		map_t _map;

	public:
		using parallel_encoded_data_t = std::vector<data_t>;

		encoder() {
			for (int i = 0; i < 0xff; i++) {
				_map.emplace(std::string(1, static_cast<char>(i)), i);
			}
		}

		encoder(const encoder &object) = default;
		encoder(encoder &&object) = default;
		encoder &operator=(const encoder &object) = default;
		encoder &operator=(encoder &&object) = default;
		~encoder() = default;

		template <class InputIter> data_t encode(InputIter begin, InputIter end) {
			// init block
			Key previous;   // previous character/characters
			Key current;    // current character/characters
			data_t encoded; // main output -- encoded string

			// main cycle
			while (begin != end) {
				current = *begin;
				if (_map.find(previous + current) !=
					_map.end()) { // if we find pair of character in hashtable
					previous += current;
				}
				else { // if we don't find
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
		static std::vector<data_t> parallel_encode(const RandomAccessIter begin,
			const RandomAccessIter end) {
			using future_t = std::future<data_t>;
			const auto size = end - begin;

			// FIXME: no need for n threads if size is small
			// FIXME: if size < nthreads then size_per_thread = 0
			const auto nthreads = std::thread::hardware_concurrency();
			const auto size_per_thread = size / nthreads;

			std::vector<future_t> futures;
			for (unsigned int i = 0; i < nthreads - 1; i++) {
				futures.emplace_back(std::async(
					[start = begin + i * size_per_thread, size_per_thread]() {
					encoder local_encoder;
					return local_encoder.encode(start, start + size_per_thread);
				}));
			}

			futures.emplace_back(
				std::async([start = begin + (nthreads - 1) * size_per_thread, end]() {
				encoder local_encoder;
				return local_encoder.encode(start, end);
			}));

			std::vector<data_t> encoded_data(nthreads);
			for (auto &&future : futures) {
				if (future.valid()) {
					encoded_data.emplace_back(future.get());
				}
				else {
					throw std::runtime_error("Something going wrong.");
				}
			}
			return encoded_data;
		}
	};
}
#endif // ENCODER_HPP
