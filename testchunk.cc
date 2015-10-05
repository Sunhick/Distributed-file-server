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
  if (argc < 2) {
    std::cout << "Error in invoking...!" << std::endl;
    return 1;
  }
  
  try {
    df_chunk_srv srv(argv[1], atoi(argv[2]));
    srv.listen_forever();
  } catch (...) {
    cout << "ERROR " << endl;
  }

  return 0;
}
