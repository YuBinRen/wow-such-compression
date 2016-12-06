#pragma warning( disable :4996)
#include <iostream>
#include <fstream>
#include "encoder.hpp"
#include "decoder.hpp"
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/device/file.hpp>

using parallel_encoder_type = lzw::encoder<char>::parallel_encoder_type;
namespace io = boost::iostreams;

void write_file_encode(const std::string& file_path_output, std::vector<uint16_t>& encoded)
{
	io::file_sink output_stream(file_path_output, std::ios::out);
	output_stream.write(reinterpret_cast<char*>(encoded.data()), encoded.size() * sizeof(encoded[0]));
	output_stream.flush();
}

void write_file_parallel_encoded(const std::string& file_path_output, parallel_encoder_type encoded)
{
	io::file_sink output_stream(file_path_output, std::ios::out);
	for (const auto &encode_part : encoded)
	{
		for (const auto &value: encode_part)
		{
			if (value == '\n' || value == '\\')
			{
				io::put(output_stream, '\\');
			}
			io::put(output_stream, value);
		}
		io::put(output_stream, '\n');
	}

}

void write_file_decode(const std::string& file_path_output, std::vector<char>& decoded)
{
	io::file_sink outoutput_stream(file_path_output, std::ios::out);
	outoutput_stream.write(decoded.data(), decoded.size() * sizeof(decoded[0]));
}

//boost file system mapped file
int main()
{
	 std::string file_path = "E:\\kursa4\\kek.lzw";
	 std::string data = "Zhat would Sonya and the count and countess have done, how would they\n\
		have looked, if nothing had been done, if there had not been those pills\n\
		to give by the clock, the warm drinks, the chicken cutlets, and all the\n\
		other details of life ordered by the doctors, the carrying out of which\n\
		supplied an occupation and consolation to the family circle?\n";
	lzw::encoder<std::string> dict_test;
	auto encoded = dict_test.encode(data.begin(), data.end());
	for (auto n : encoded)
	{
		std::cout << std::hex << n << " ";

	}
	std::cout << std::endl;
	auto encoded_parallel = dict_test.parallel_encode(data.begin(), data.end());

	write_file_parallel_encoded(file_path, encoded_parallel);
	std::cin.get();

	auto mapped_file = io::mapped_file_source(file_path);
	lzw::decoder<std::string> dict_dec;
	auto decoded_parallel = dict_dec.parallel_decode(mapped_file.data(), mapped_file.data() + mapped_file.size());
	for (const auto &chr : decoded_parallel) {
		std::cout << chr;
	}

	/*lzw::decoder<std::string> dect_decode_test;
	auto decoded = dect_decode_test.decode(encoded.begin(), encoded.end());
	std::string result(decoded.begin(), decoded.end());
	std::cout << (result == data) << data.size() << std::endl;
	std::cout << result;
	*/
	return 0;
}

