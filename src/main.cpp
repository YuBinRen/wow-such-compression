#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <stdexcept>

#include "decoder.hpp"
#include "encoder.hpp"

namespace io = boost::iostreams;

using encoder_t = lzw::encoder<std::string>;
using decoder_t = lzw::decoder<std::string>;
using encoded_data_t = lzw::encoder<char>::parallel_encoder_type;

template <class OutputStream>
void write_to_stream(OutputStream &ostream, encoded_data_t encoded) {
  for (const auto &encode_part : encoded) {
    for (const auto &value : encode_part) {
      if (value == '\n' || value == '\\') {
        io::put(ostream, '\\');
      }
      io::put(ostream, value);
    }
    io::put(ostream, '\n');
  }
}

void decode(const std::string &path) {
  io::mapped_file_source istream(path);
  auto decoded = decoder_t::parallel_decode(istream.data(),
                                            istream.data() + istream.size());
  std::cout.write(decoded.data(), decoded.size());
}

void encode(const std::string &path) {
  io::mapped_file_source istream(path);
  lzw::encoder<std::string> encoder;
  auto encoded = encoder_t::parallel_encode(istream.data(),
                                            istream.data() + istream.size());
  write_to_stream(std::cout, encoded);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cout << "Use args, Luke! \nUsage: <what_to_do> <file_path> where "
                 "what_to_do in {'-d', '-e'})"
              << std::endl;
    exit(0);
  }

  const auto what_to_do = std::string(argv[1]);
  const auto file_path = std::string(argv[2]);

  try {
    if (what_to_do == "-d") {
      decode(file_path);
    } else if (what_to_do == "-e") {
      encode(file_path);
    } else {
      std::cout << "Incorrect option, use -d or -e." << std::endl;
      std::exit(1);
    }
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }

  return 0;
}
