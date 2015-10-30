/****************************************************
 *  df utility function
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_UTILITIES_H
#define DFS_UTILITIES_H

#include <ostream>
#include <string>
#include <vector>
#include <algorithm>

#include <iostream>
#include <openssl/evp.h>
#include <openssl/aes.h>

constexpr int WRITE_SIZE = 2048 * 4;
constexpr int READ_SIZE = WRITE_SIZE;

namespace dfs {

  enum color {
    FG_RED      = 31,
    FG_GREEN    = 32,
    FG_BLUE     = 34,
    FG_CYAN     = 36,
    FG_DEFAULT  = 39,
    BG_RED      = 41,
    BG_GREEN    = 42,
    BG_BLUE     = 44,
    BG_DEFAULT  = 49
  };

  class modifier {
    color code;
  public:
  modifier(color code) : code(code) {}
    friend std::ostream& operator<<(std::ostream& os, const modifier& mod) {
      return os << "\033[" << mod.code << "m";
    }
  };

  class utilities {
  private:
    static std::string aes_encrypt(EVP_CIPHER_CTX *ctx, unsigned char *plaintext, int *len);
    static std::string aes_decrypt(EVP_CIPHER_CTX *ctx, unsigned char *ciphertext, int *len);

  public:
    static std::vector<std::string> split(const std::string& str, int delimiter(int),
					  int occurances = -1); // -1: look for all occurances

    static int aes_init(unsigned char *key_data, int key_data_len, unsigned char *salt, EVP_CIPHER_CTX *e_ctx, 
		 EVP_CIPHER_CTX *d_ctx);

    // vanilla flavors for ease of use
    static std::string aes_encrypt(const std::string& plaintext, const std::string& password);
    static std::string aes_decrypt(const std::string& ciphertext, const std::string& password);
  };
}

#endif
