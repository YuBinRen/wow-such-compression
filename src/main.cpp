#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <stdexcept>

#include "decoder.hpp"
#include "encoder.hpp"

namespace io = boost::iostreams;

using encoder_t = lzw::encoder<std::string>;
using decoder_t = lzw::decoder<std::string>;
using encoded_data_t = lzw::encoder<char>::parallel_encoded_data_t;

template <class OutputStream>
void write_to_stream(OutputStream &ostream, encoded_data_t encoded) {
  for (const auto &encoded_part : encoded) {
    const char *const begin =
        reinterpret_cast<const char *>(encoded_part.data());
    const size_t size = sizeof(encoded_part[0]) * encoded_part.size();
    const char *const end = begin + size;

    for (const char *current = begin; current != end; ++current) {
      if (*current == '\n' || *current == '\\') {
        io::put(ostream, '\\');
      }
      io::put(ostream, *current);
    }
    io::put(ostream, '\n');
  }
}

void decode(const std::string &path) {
  io::mapped_file_source istream(path);
  auto decoded = decoder_t::parallel_decode(istream.data(),
                                            istream.data() + istream.size());
  std::cout.write(decoded.data(), static_cast<std::streamsize>(decoded.size()));
}

void encode(const std::string &path) {
  io::mapped_file_source istream(path);
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
      std::cout << "Incorrect option. Use -d or -e." << std::endl;
      std::exit(1);
    }
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }

  return 0;
}
