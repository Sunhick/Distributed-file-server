/****************************************************
 *  df tester file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/

#include "include/dfconfig.h"

#include <iostream>

using namespace dfs;
using namespace std;

int main(int argc, char *argv[])
{
  try {
    dfconfig c("dfc.conf");
    auto addr = c.get_chunk_server("DFS1");
    cout << addr.ip_address << "--" << addr.port << "\n";
    cout << "Auth: " << c.validate("Alice", "-SimplePassword") << endl;
  } catch (const char* msg) {
    cout << msg  << "\n";    
  }
  return 0;
}

