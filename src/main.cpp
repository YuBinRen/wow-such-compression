#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <stdexcept>

#include "decoder.hpp"
#include "encoder.hpp"

namespace io = boost::iostreams;

using encoder_t = lzw::encoder;
using decoder_t = lzw::decoder;
using encoded_data_t = lzw::encoder::parallel_encoded_data_t;

template <class OutputStream>
void write_to_stream(OutputStream &ostream, encoded_data_t encoded) {
  for (const auto &encoded_part : encoded) {
    const uint16_t *const begin = encoded_part.data();
    const uint16_t *const end = begin + encoded_part.size();
    static const uint16_t BACK_SLASH = '\\';
    static const uint16_t NEW_LINE = '\n';
    for (const uint16_t *current = begin; current != end; ++current) {
      if (*current == '\n' || *current == '\\') {
        assert(io::write(ostream, reinterpret_cast<const char *>(&BACK_SLASH),
                         sizeof(uint16_t)) == sizeof(uint16_t));
        // io::put(ostream, '\\');
        // io::put(ostream, '\0');
      }
      assert(io::write(ostream, reinterpret_cast<const char *>(current),
                       sizeof(uint16_t)) == sizeof(uint16_t));
    }
    assert(io::write(ostream, reinterpret_cast<const char *>(&NEW_LINE),
                     sizeof(uint16_t)) == sizeof(uint16_t));
    // io::put(ostream, '\n');
    // io::put(ostream, '\0');
  }
}

static void decode(const std::string &path) {
  io::mapped_file_source file(path);
  auto decoded = decoder_t::parallel_decode(
      reinterpret_cast<const uint16_t *>(file.data()),
      reinterpret_cast<const uint16_t *>(file.data() + file.size()));
  assert(decoded.size() != 0);
  std::cout.write(decoded.data(), static_cast<std::streamsize>(decoded.size()));
  std::cout << std::flush;
}

static void encode(const std::string &path) {
  io::mapped_file_source file(path);
  auto encoded =
      encoder_t::parallel_encode(file.data(), file.data() + file.size());
  write_to_stream(std::cout, encoded);
}
static void encode_single(const std::string &path) {
  io::mapped_file_source file(path);
  auto encoder = encoder_t();
  auto encoded = encoder.encode(file.data(), file.data() + file.size());
  std::cout.write(
      reinterpret_cast<const char *>(encoded.data()),
      static_cast<std::streamsize>((encoded.size() * sizeof(encoded[0]))));
}

static void decode_single(const std::string &path) {
  io::mapped_file_source file(path);
  auto decoder = decoder_t();
  auto decoded = decoder.decode(
      reinterpret_cast<const uint16_t *>(file.data()),
      reinterpret_cast<const uint16_t *>(file.data() + file.size()));
  std::cout.write(decoded.data(), static_cast<std::streamsize>(decoded.size()));
}

static const std::string TEXT =
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

static void iter_test() {
  std::string input = "\\\\ \\\n \\ \n";
  std::string expected = "\\ \n  \n";
  auto first = lzw::unescape(input.begin());
  auto last = lzw::unescape(input.end());
  assert(std::equal(first, last, expected.begin(), expected.end()));
}

static void threading_decode_test() {
  const auto encoded = encoder_t::parallel_encode(TEXT.begin(), TEXT.end());
  const auto encoded_total = [&encoded]() {
    std::vector<uint16_t> total;
    for (const auto &part : encoded) {
      total.insert(total.end(), part.begin(), part.end());
    }
    return total;
  }();
  const auto decoded = decoder_t::parallel_decode(
      encoded_total.data(), encoded_total.data() + encoded_total.size());
  std::cout << std::string(decoded.begin(), decoded.end());
  assert(TEXT == std::string(decoded.begin(), decoded.end()));
}

static void decode_test() {
  auto encoder = encoder_t();
  auto encoded = encoder.encode(TEXT.begin(), TEXT.end());
  auto decoder = decoder_t();
  auto decoded = decoder.decode(encoded.begin(), encoded.end());
  assert(TEXT == std::string(decoded.begin(), decoded.end()));
}

int main(int argc, char *argv[]) {
  // TODO: Replace this with boost library that make the same shit
  if (argc != 3) {
    std::cout << "Use args, Luke! \nUsage: <what_to_do> <file_path> where "
                 "what_to_do in {'-d', '-e', '-ds', '-es', '-ts', '-t'})"
              << std::endl;
    exit(0);
  }

  const auto what_to_do = std::string(argv[1]);
  const auto file_path = std::string(argv[2]);

  // try {
  if (what_to_do == "-d") {
    decode(file_path);
  } else if (what_to_do == "-e") {
    encode(file_path);
  } else if (what_to_do == "-ds") {
    decode_single(file_path);
  } else if (what_to_do == "-es") {
    encode_single(file_path);
  } else if (what_to_do == "-ts") {
    iter_test();
    decode_test();
  } else if (what_to_do == "-t") {
    threading_decode_test();
  } else {
    std::cerr << "Incorrect option. Use -d or -e." << std::endl;
    std::exit(1);
  }
  //}

  // catch (const std::exception &e) {
  // std::cerr << "Error: " << e.what() << std::endl;
  //}

  return 0;
}
