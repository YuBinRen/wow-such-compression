#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/program_options.hpp>
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
      }
      assert(io::write(ostream, reinterpret_cast<const char *>(current),
                       sizeof(uint16_t)) == sizeof(uint16_t));
    }
    assert(io::write(ostream, reinterpret_cast<const char *>(&NEW_LINE),
                     sizeof(uint16_t)) == sizeof(uint16_t));
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

int main(int argc, char *argv[]) {
  using option_description_t = boost::program_options::options_description;
  using boost::program_options::command_line_parser;
  using variables_map_t = boost::program_options::variables_map;

  option_description_t gen_options("General options");
  gen_options.add_options()
      ("help,h", "Show help")
      ("encode,e", boost::program_options::value<std::string>(), "Encode file using several threads")
      ("encode-single,s", boost::program_options::value<std::string>(), "Encode file using one thread")
      ("decode,d", boost::program_options::value<std::string>(), "Decode file using that much threads as we using at encode statement")
      ("decode-single,z", boost::program_options::value<std::string>(), "Decode file using one thread");

  variables_map_t vm;
  auto parsed = command_line_parser(argc, argv).options(gen_options).run();
  boost::program_options::store(parsed, vm);
  boost::program_options::notify(vm);

  if (vm.count("encode")){
    encode(vm["encode"].as<std::string>());
  }
  else if (vm.count("encode-single")){
    encode_single(vm["encode-single"].as<std::string>());
  }
  else if (vm.count("decode")){
    decode(vm["decode"].as<std::string>());
  }
  else if (vm.count("decode-single")){
    decode_single(vm["decode-single"].as<std::string>());
  }
  else {
    std::cout << gen_options;
  }
  return 0;
}
