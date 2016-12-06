#ifndef DECODER_HPP
#define DECODER_HPP
#include <vector>
#include <cassert>


namespace lzw
{
	template <class Value>
	class decoder
	{
		//wanted to use std::vector<char>, but this structure doesn't has the hash function in unordered map :(
		using map_type = std::vector<Value>;
	private:
		map_type _map;
	public:
		decoder()
		{
			for (auto i = 0; i < 0xff; i++)
			{
				_map.emplace_back(1, i);
			}
			assert(_map.size() == 0xff);
		}

		decoder(const decoder& object) = default;
		decoder(decoder&& object) = default;
		decoder& operator=(const decoder& object) = default;
		decoder& operator=(decoder&& object) = default;
		~decoder() = default;

		template <class InputIter>
		std::vector<char> decode(InputIter begin, InputIter end)
		{
			uint16_t current_code = *begin;
			uint16_t previous_code = current_code;
			Value previous;
			Value current;
			assert(current_code < _map.size());
			std::vector<char> decoded = {_map[current_code][0]}; //main output -- decoded file

			++begin;
			while (begin != end)
			{
				current_code = *begin;
				if (current_code < _map.size())
				{
					const auto& temp = _map[current_code];
					decoded.insert(decoded.end(), temp.begin(), temp.end());
					assert(previous_code < _map.size());
					previous = _map[previous_code];
					current = std::string(1, temp[0]);
					_map.emplace_back(previous + current);
				}
				else
				{
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

		//идем по файлы до первого неэкраннированного \n, делаем тред, ему отдаем этот интервал и так до конца
		template <class RandomAccessIter>
		static std::vector<char> parallel_decode(RandomAccessIter begin, RandomAccessIter end)
		{

		}

	};
}
#endif // DECODER_HPP
