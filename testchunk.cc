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
  try {
    df_chunk_srv srv("DFS2", 10001);
    srv.list();
  } catch (...) {
    cout << "ERROR " << endl;
  }

  return 0;
}
