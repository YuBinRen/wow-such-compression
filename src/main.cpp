#pragma warning(disable : 4996)
#include "decoder.hpp"
#include "encoder.hpp"
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <fstream>
#include <iostream>

// boost file system mapped file
int main() {

  std::string data =
      "Zhat would Sonya and the count and countess have done, how would they\n\
		have looked, if nothing had been done, if there had not been those pills\n\
		to give by the clock, the warm drinks, the chicken cutlets, and all the\n\
		other details of life ordered by the doctors, the carrying out of which\n\
		supplied an occupation and consolation to the family circle ?";
  lzw::encoder<std::string> dict_test;
  auto encoded = dict_test.encode(data.begin(), data.end());
  for (auto n : encoded) {
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
