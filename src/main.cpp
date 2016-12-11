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
        io::put(ostream, '\0');
      }
      io::put(ostream, *current);
    }
    io::put(ostream, '\n');
    io::put(ostream, '\0');
  }
}

void decode(const std::string &path) {
  io::mapped_file_source istream(path);
  auto decoded = decoder_t::parallel_decode(
      reinterpret_cast<const uint16_t *>(istream.data()),
      reinterpret_cast<const uint16_t *>(istream.data()) +
          istream.size() / sizeof(uint16_t));
  std::cout.write(decoded.data(), static_cast<std::streamsize>(decoded.size()));
}

void encode(const std::string &path) {
  io::mapped_file_source istream(path);
  auto encoded = encoder_t::parallel_encode(istream.data(),
                                            istream.data() + istream.size());
  write_to_stream(std::cout, encoded);
}
// TODO: istream -> file
void encode_single(const std::string &path) {
  io::mapped_file_source istream(path);
  auto encoder = encoder_t();
  auto encoded =
      encoder.encode(istream.data(), istream.data() + istream.size());
  std::cout.write(reinterpret_cast<const char *>(encoded.data()),
                  encoded.size() * sizeof(encoded[0]));
}

void decode_single(const std::string &path) {
  io::mapped_file_source istream(path);
  auto decoder = decoder_t();
  auto decoded =
      decoder.decode(reinterpret_cast<const uint16_t *>(istream.data()),
                     reinterpret_cast<const uint16_t *>(istream.data()) +
                         istream.size() / sizeof(uint16_t));
  std::cout.write(decoded.data(), static_cast<std::streamsize>(decoded.size()));
}

void iter_test() {
  std::string input = "\\\\ \\\n \\ \n";
  std::string expected = "\\ \n  \n";
  auto first = lzw::unescape(input.begin());
  auto last = lzw::unescape(input.end());
  assert(std::equal(first, last, expected.begin(), expected.end()));
}

void decode_test() {
  std::string input =
      "The Project Gutenberg EBook of War and Peace, by Leo Tolstoy\n"
      "This eBook is for the use of anyone anywhere at no cost and with "
      "almost\n"
      "no restrictions whatsoever. You may copy it, give it away or re-use\n"
      "it under the terms of the Project Gutenberg License included with this\n"
      "eBook or online at www.gutenberg.org\n"
      "Title: War and Peace\n"
      "Author: Leo Tolstoy\n"
      "Translators: Louise and Aylmer Maude\n"
      "Posting Date: January 10, 2009 [EBook #2600] Last Updated: November 3,\n"
      "2016\n"
      "Language: English\n"
      "Character set encoding: UTF-8\n"
      "*** START OF THIS PROJECT GUTENBERG EBOOK WAR AND PEACE ***\n";
  auto encoder = encoder_t();
  auto encoded = encoder.encode(input.begin(), input.end());
  auto decoder = decoder_t();
  auto decoded = decoder.decode(encoded.begin(), encoded.end());
  assert(input == std::string(decoded.begin(), decoded.end()));
}

int main(int argc, char *argv[]) {
  // TODO: WARNING!!!! REMOVE DAMN TEMPLATES LEEEEEEEEEEL
  // TODO: Replace this with boost library that make the same shit
  if (argc != 3) {
    std::cout << "Use args, Luke! \nUsage: <what_to_do> <file_path> where "
                 "what_to_do in {'-d', '-e', '-ds', '-es', '-t'})"
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
    } else if (what_to_do == "-ds") {
      decode_single(file_path);
    } else if (what_to_do == "-es") {
      encode_single(file_path);
    } else if (what_to_do == "-t") {
      iter_test();
      decode_test();
    } else {
      std::cout << "Incorrect option. Use -d or -e." << std::endl;
      std::exit(1);
    }
  }

  catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }

  return 0;
}
