#ifndef PROTECTOR_HPP
#define PROTECTOR_HPP
#include <cstdint>
#include <string>

class protector {
public:
  static bool has_access();
  static void make_licence();
  static bool check_key(const std::string &licence_key);
  protector();
  ~protector();
  void aes_decrypt(uint8_t *ciphertext);
  void aes_encrypt(uint8_t *plaintext);
  // private:
};

#endif // PROTECTOR_HPP
