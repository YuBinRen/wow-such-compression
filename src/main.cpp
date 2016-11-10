#pragma warning( disable :4996)
#include <iostream>
#include <fstream>
#include "encoder.hpp"
#include "decoder.hpp"
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/device/file.hpp>

//boost file system mapped file
int main()
{
	/*
	const std::string file_path_output = "C:\\Users\\pl0q1n\\Documents\\Visual Studio 2015\\Projects\\lzw_damn\\x64\\Debug\\xoi.lzw";
	
	
	
	const std::string file_path = "C:\\Users\\pl0q1n\\Documents\\Visual Studio 2015\\Projects\\lzw_damn\\x64\\Debug\\lzw_damn.ilk";
	boost::iostreams::mapped_file_source input_stream(file_path);
	lzw::encoder<std::string> encoder;
	
	auto encoded = encoder.encode(input_stream.data(),
	                                input_stream.data() + input_stream.size());

	{

	boost::iostreams::file_sink output_stream(file_path_output, std::ios::out);
	output_stream.write(reinterpret_cast<char*>(encoded.data()), encoded.size() * sizeof(encoded[0]));
	std::cout << encoded.size() * sizeof(encoded[0]) << std::endl;
	output_stream.flush();
	}
	

	
	{
	const std::string file_path = "C:\\Users\\pl0q1n\\Documents\\Visual Studio 2015\\Projects\\lzw_damn\\x64\\Debug\\wac_decoded.txt";

	boost::iostreams::mapped_file_source input_stream_decode(file_path_output);
	std::cout << input_stream_decode.size() << std::endl;
	auto ptr = reinterpret_cast<const uint16_t*>(input_stream_decode.data());
	auto size = input_stream_decode.size() / sizeof(uint16_t);
	assert(input_stream_decode.size() % sizeof(uint16_t) == 0);
	assert(size == encoded.size());
	assert(std::equal(encoded.begin(), encoded.end(), ptr));
	lzw::decoder<std::string> decoder;
	auto decode = decoder.decode(ptr, ptr + size);
	boost::iostreams::file_sink outoutput_stream(file_path, std::ios::out);
	outoutput_stream.write(decode.data(), decode.size() * sizeof(decode[0]));
	}
	
	*/




	
	 std::string data = "Zhat would Sonya and the count and countess have done, how would they\n\
		have looked, if nothing had been done, if there had not been those pills\n\
		to give by the clock, the warm drinks, the chicken cutlets, and all the\n\
		other details of life ordered by the doctors, the carrying out of which\n\
		supplied an occupation and consolation to the family circle ?";
	lzw::encoder<std::string> dict_test;
	auto encoded = dict_test.encode(data.begin(), data.end());
	for (auto n : encoded)
	{
		std::cout << std::hex << n << " ";

	}
	std::cout << std::endl;
	lzw::decoder<std::string> dect_decode_test;
	auto decoded = dect_decode_test.decode(encoded.begin(), encoded.end());
	std::string result(decoded.begin(), decoded.end());
	std::cout << (result == data) << data.size() << std::endl;
	std::cout << result;
	std::cin.get();

	return 0;
}

