/****************************************************
 *  df tester file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/

#include "include/dfconfig.h"
#include "include/dfutils.h"

#include <string>
#include <iostream>

using namespace dfs;
using namespace std;

int main(int argc, char *argv[])
{
  try {

    std::string pass = "password";
    std::string data = "This is a simple test phrase\n\t12334567890-~!@#$%^&*()_+";
    auto encrypt = utilities::aes_encrypt(data, pass);
    auto decrypt = utilities::aes_decrypt(encrypt, pass);
    std::cout << decrypt << std::endl;

    dfconfig c("dfc.conf");
    auto addr = c.get_chunk_server("DFS1");
    cout << addr.ip_address << "--" << addr.port << "\n";
    cout << "Auth: " << c.validate("Alice", "-SimplePassword") << endl;
  } catch (const char* msg) {
    cout << msg  << "\n";    
  }
  return 0;
}

