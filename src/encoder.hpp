#ifndef ENCODER_HPP
#define ENCODER_HPP
#include <unordered_map>

namespace lzw
{
	template <class Key>
	class encoder
	{
		//wanted to use std::vector<char>, but this structure doesn't has the hash function in unordered map :( 
		using map_type = std::unordered_map<Key, uint16_t>;

		map_type _map;
	public:
		encoder()
		{
			for (int i = 0; i < 0xff; i++)
			{
				_map.emplace(std::string(1, i), i);
			}
		}

		encoder(const encoder& object) = default;
		encoder(encoder&& object) = default;
		encoder& operator=(const encoder& object) = default;
		encoder& operator=(encoder&& object) = default;
		~encoder() = default;

		template <class InputIter>
		std::vector<uint16_t> encode(InputIter begin, InputIter end)
		{
			//init block
			Key previous; // previous character/characters
			Key current; // current character/characters
			std::vector<uint16_t> encoded; // main output -- encoded string

			//main cycle
			while (begin != end)
			{
				current = *begin;

				if (_map.find(previous + current) != _map.end())
				{ // if we find pair of character in hashtable
					previous += current;
				}
				else
				{ // if we don't find 
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
	};


	// dictionary init_map(const char* input, std::size_t count);

	//	std::pair<dictionary, std::vector<uint16_t>> encode(const char* input, size_t input_size);
}
#endif //ENCODER_HPP


