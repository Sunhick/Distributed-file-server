/****************************************************
 *  Distributed file chunk server header file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_CHUNK_SRV_H
#define DFS_CHUNK_SRV_H

#include <string>

namespace dfs {
  class df_chunk_srv {
  private:
    std::string server_id;
    std::string filesys = "/DFS#";
    int port = -1;
    
  public:
    df_chunk_srv(std::string filesys, int port);
    ~df_chunk_srv();

    // list all available files under the server
    void list();
    // get the piece information from the server
    void get(std::string filename);
    // store the chunk file in server
    void put(std::string filename, std::string content);
  };
}

#endif
