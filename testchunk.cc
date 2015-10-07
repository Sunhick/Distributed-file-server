/****************************************************
 *  df chunk server test
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfchunksrv.h"

#include <iostream>

using namespace std;
using namespace dfs;

int main(int argc, char *argv[])
{
  if (argc < 3) {
    std::cout << "Invalid parameters. pass file system and port!" << std::endl;
    return 1;
  }
  
  try {
    df_chunk_srv srv(argv[1], atoi(argv[2]));
    srv.listen_forever();
  } catch (char* msg) {
    cout << "ERROR:" << msg << std::endl;
  } catch (...) {
    cout << "ERROR " << endl;
  }

  return 0;
}
