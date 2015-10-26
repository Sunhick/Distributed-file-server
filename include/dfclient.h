/****************************************************
 *  df client header file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_CLIENT_H
#define DFS_CLIENT_H

#include <string>
#include <map>
#include <vector>

#include "dfcomm.h"
#include "dfconfig.h"
#include "dfproto.h"

namespace dfs {
  // df client
  class df_client {
  private:
    // map of <server id, communication channel>
    std::map<std::string, generic_comm*> channels;
    dfconfig* config;
    std::vector<int> sockfds;
    std::map<int, std::vector<struct upload_policy>> upload_policies;
    
    // df request protocol. re-use the packet
    // declare once and use multiple times by changing the command
    df_request_proto* request;

    // list all files available under all servers
    void list();

    // get the specified file from available server
    void get();

    // put the specified file under the given server based
    // the traffic etc
    void put();

    // make user directory
    void mkdir();
    
    // wait for server time out
    bool server_timeout(int filedesc);

    // the policy number which represents where to store piece of
    // file in which server
    int get_policy(std::string file);
    void init_upload_policies();
    bool get_filename_chunknum(const std::string& msg,
			       std::string& filename,
			       int& chunk) const;
  public:
    df_client(std::string& file);
    ~df_client();
    void start();
  };

  struct upload_policy {
    std::string name;	//server name
    std::vector<int> chunk_ids;
  };
}
#endif
