/****************************************************
 *  df utility function
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfutils.h"
#include <string.h>

using namespace dfs;

std::vector<std::string> utilities::split(const std::string& str, int delimiter(int), int occurances)
{
  std::vector<std::string> result;
  auto end = str.end();
  auto start = str.begin();
  while (start != end && occurances) {
    occurances--;
    start = find_if_not(start, end, delimiter);
    if (start == end) break;
    auto index = find_if(start, end, delimiter);
    if (occurances == 0)
      result.push_back(std::string(start--, end));
    else
      result.push_back(std::string(start, index));
    start = index;
  }

  return result;
}

int utilities::aes_init(unsigned char *key_data, int key_data_len, unsigned char *salt, EVP_CIPHER_CTX *e_ctx, 
	     EVP_CIPHER_CTX *d_ctx)
{
  int i, nrounds = 5;
  unsigned char key[32], iv[32];
     
  i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt, key_data, key_data_len, nrounds, key, iv);
  if (i != 32) {
    std::cout << "Key size is " << i << " bits - should be 256 bits\n";
    return -1;
  }
     
  EVP_CIPHER_CTX_init(e_ctx);
  EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
  EVP_CIPHER_CTX_init(d_ctx);
  EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, key, iv);
     
  return 0;
}

std::string utilities::aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len)
{
  int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
  unsigned char *ciphertext = new unsigned char[c_len];

  EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);
  EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);     
  EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);
     
  *len = c_len + f_len;
  return std::string(ciphertext, ciphertext + *len);
}

std::string utilities::aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len)
{
  int p_len = *len, f_len = 0;
  unsigned char *plaintext = new unsigned char[p_len];
     
  EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
  EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
  EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);
     
  *len = p_len + f_len;
  return std::string(plaintext, plaintext + *len);
}

std::string utilities::aes_encrypt(const std::string& plaintext, const std::string& password)
{
  EVP_CIPHER_CTX en, de;
     
  unsigned int salt[] = { 12345, 54321 };
  unsigned char *key_data;
  const char *input = plaintext.c_str();

  key_data = (unsigned char *)password.c_str();
  int key_data_len = password.size();

  if (utilities::aes_init(key_data, key_data_len, (unsigned char *)&salt, &en, &de)) {
    std::cout << "Couldn't initialize AES cipher\n";
    return std::string("");
  }

  int len = strlen(input) + 1;
  auto ciphertext = utilities::aes_encrypt(&en, (unsigned char *)input, &len);
  return ciphertext;
}

std::string utilities::aes_decrypt(const std::string& ciphertext, const std::string& password)
{
  EVP_CIPHER_CTX en, de;
  unsigned int salt[] = { 12345, 54321 };
  unsigned char *key_data;
  const char *input = ciphertext.c_str();
     
  key_data = (unsigned char *)password.c_str();
  int key_data_len = password.size();

  if (utilities::aes_init(key_data, key_data_len, (unsigned char *)&salt, &en, &de)) {
    std::cout << "Couldn't initialize AES cipher\n";
    return std::string("");
  }

  int len = strlen(input);
  auto plaintext = utilities::aes_decrypt(&de, (unsigned char*)input, &len);
  return plaintext;
}
