#include "protector.hpp"

#include <algorithm>
#include <string>

// For debugging
#include <iostream>

#include <boost/iostreams/device/mapped_file.hpp>

#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/evp.h>

const static std::string path = "guard.urmom";

const static std::array<uint8_t, 256> key = {
    123, 33,  142, 116, 180, 183, 28,  111, 217, 239, 177, 61,  5,   74,  174,
    164, 188, 211, 72,  35,  144, 180, 53,  7,   160, 67,  2,   54,  5,   166,
    16,  132, 181, 136, 19,  94,  16,  237, 194, 189, 71,  115, 71,  175, 111,
    163, 210, 182, 76,  34,  94,  199, 240, 84,  105, 142, 247, 82,  219, 238,
    10,  150, 198, 220, 222, 196, 220, 4,   42,  170, 99,  195, 209, 61,  76,
    169, 255, 163, 81,  158, 242, 219, 229, 48,  199, 42,  244, 71,  11,  191,
    93,  0,   217, 212, 53,  74,  222, 154, 51,  91,  212, 48,  54,  29,  234,
    201, 169, 35,  194, 182, 144, 72,  71,  136, 85,  73,  105, 129, 16,  95,
    20,  68,  224, 101, 127, 44,  118, 20,  222, 66,  0,   234, 170, 67,  52,
    114, 45,  76,  27,  51,  131, 171, 209, 6,   141, 102, 131, 243, 100, 233,
    100, 100, 117, 112, 190, 109, 199, 45,  86,  67,  57,  19,  22,  152, 117,
    239, 140, 51,  228, 27,  132, 51,  212, 135, 99,  157, 253, 47,  177, 121,
    76,  21,  123, 238, 132, 0,   62,  150, 151, 202, 242, 194, 249, 254, 128,
    105, 160, 126, 2,   217, 189, 235, 89,  69,  168, 214, 216, 75,  43,  116,
    42,  61,  221, 15,  59,  168, 70,  201, 20,  71,  9,   125, 16,  246, 53,
    94,  26,  189, 37,  4,   85,  68,  244, 125, 246, 104, 227, 249, 40,  86,
    24,  118, 192, 6,   191, 59,  56,  251, 92,  143, 75,  190, 27,  198, 109,
    72};

protector::protector() {
  ERR_load_crypto_strings();
  OpenSSL_add_all_algorithms();
  OPENSSL_config(NULL);
}

void protector::make_licence() {
  int32_t access_value = -1;
  protector guard;
  std::array<uint8_t, 256> buffer;
  std::fill(reinterpret_cast<int32_t *>(buffer.data()),
            reinterpret_cast<int32_t *>(buffer.data() + buffer.size()),
            access_value);

  guard.aes_encrypt(buffer.data());
  boost::iostreams::mapped_file file(path);
  std::copy(buffer.data(), buffer.data() + buffer.size(), file.data());
}

protector::~protector() {
  EVP_cleanup();
  ERR_free_strings();
}

bool protector::check_key(const std::string &licence_key) {
  if (licence_key.size() != 32) {
    return false;
  }
  std::array<uint8_t, 16> buffer;
  try {
    for (auto i = 0; i < buffer.size(); i++) {
      buffer[i] = std::stoi(licence_key.substr(i * 2, 2), nullptr, 16);
    }
    auto result =
        std::search(key.begin(), key.end(), buffer.begin(), buffer.end());
    return result != key.end();
  } catch (...) {
    return false;
  }
}

bool protector::has_access() {
  protector guard;
  boost::iostreams::mapped_file file(path);
  if (file.size() != key.size()) {
    return false;
  }

  std::array<uint8_t, 256> buffer;
  std::copy(file.data(), file.data() + file.size(), buffer.begin());
  guard.aes_decrypt(buffer.data());
  int32_t access_value = *reinterpret_cast<int32_t *>(buffer.data());

  if (std::any_of(
          reinterpret_cast<int32_t *>(buffer.data()),
          reinterpret_cast<int32_t *>(buffer.data() + buffer.size()),
          [access_value](auto value) { return value != access_value; })) {
    return false;
  }

  if (access_value == -1) {
    return true;
  }

  if (access_value <= 0) {
    return false;
  }

  access_value--;
  std::fill(reinterpret_cast<int32_t *>(buffer.data()),
            reinterpret_cast<int32_t *>(buffer.data() + buffer.size()),
            access_value);

  guard.aes_encrypt(buffer.data());
  std::copy(buffer.data(), buffer.data() + buffer.size(), file.data());

  return true;
}

void protector::aes_encrypt(uint8_t *plaintext) {
  EVP_CIPHER_CTX ctx;

  std::array<uint8_t, 256> ciphertext;
  int len;
  /* Create and initialise the context */

  EVP_CIPHER_CTX_init(&ctx);

  // if (ctx == NULL)
  //  assert(false);

  /* Initialise the encryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if (1 != EVP_EncryptInit_ex(&ctx, EVP_aes_256_ecb(), NULL, key.data(), NULL))
    assert(false);

  /* Provide the message to be encrypted, and obtain the encrypted output.
   * EVP_EncryptUpdate can be called multiple times if necessary
   */
  if (1 != EVP_EncryptUpdate(&ctx, ciphertext.data(), &len, plaintext, 256))
    assert(false);
  /* Finalise the encryption. Further ciphertext bytes may be written at
   * this stage.
   */

  // if (1 != EVP_EncryptFinal_ex(&ctx, ciphertext.data() + len, &len))
  // assert(false);

  /* Clean up */

  EVP_CIPHER_CTX_cleanup(&ctx);

  std::copy(ciphertext.data(), ciphertext.data() + ciphertext.size(),
            plaintext);
}

void protector::aes_decrypt(uint8_t *ciphertext) {
  EVP_CIPHER_CTX ctx;

  int len;
  std::array<uint8_t, 256> plaintext;

  /* Create and initialise the context */

  EVP_CIPHER_CTX_init(&ctx);
  // if (!(ctx = EVP_CIPHER_CTX_new()))
  // assert(false);

  /* Initialise the decryption operation. IMPORTANT - ensure you use a key
   * and IV size appropriate for your cipher
   * In this example we are using 256 bit AES (i.e. a 256 bit key). The
   * IV size for *most* modes is the same as the block size. For AES this
   * is 128 bits */
  if (1 != EVP_DecryptInit_ex(&ctx, EVP_aes_256_ecb(), NULL, key.data(), NULL))
    assert(false);

  /* Provide the message to be decrypted, and obtain the plaintext output.
   * EVP_DecryptUpdate can be called multiple times if necessary
   */
  if (1 != EVP_DecryptUpdate(&ctx, plaintext.data(), &len, ciphertext, 256))
    assert(false);

  /* Finalise the decryption. Further plaintext bytes may be written at
   * this stage.
   */
  // if (1 != EVP_DecryptFinal_ex(&ctx, plaintext.data() + plaintext.size(),
  // &len))
  // assert(false);

  /* Clean up */
  EVP_CIPHER_CTX_cleanup(&ctx);
  std::copy(plaintext.data(), plaintext.data() + plaintext.size(), ciphertext);
}
