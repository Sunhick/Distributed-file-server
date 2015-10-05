/****************************************************
 *  df client header file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_CLIENT_H
#define DFS_CLIENT_H

#include <string>
#include <map>

#include "dfcomm.h"
#include "dfconfig.h"

namespace dfs {
  // df client
  class df_client {
  private:
    // map of <server id, communication channel>
    std::map<std::string, generic_comm*> channels;
    dfconfig* config;
    std::vector<int> sockfds;

    // list all files available under all servers
    void list();
    // get the specified file from available server
    void get();
    // put the specified file under the given server based
    // the traffic etc
    void put();
    
  public:
    df_client(std::string& file);
    ~df_client();
    void start();
  };
}
#endif
