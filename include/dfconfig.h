/****************************************************
 *  df configuration parser header file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_DFCONFIG_H
#define DFS_DFCONFIG_H

#include <map>
#include <string>
#include <vector>

namespace dfs {

  // address of DF chunk server. Its' composed of Ip address and port
  struct dfs_address {
    std::string ip_address;
    int port;
  };

  // df configuration parser
  class dfconfig {
  private:
    // map of <dfs_name, dfs_address>
    std::map<std::string, dfs_address> chunk_servers;
    // map of users and their passwords for validation
    std::map<std::string, std::string> authentications;
    
  public:
    dfconfig(std::string fpath);
    ~dfconfig();

    std::map<std::string, dfs_address> get_all_servers();
    dfs_address get_chunk_server(std::string name);
    bool validate(std::string name, std::string password);
    // index represents the entry index which will specify which
    // user credentials to use.
    void get_username_password(std::string& name,
			       std::string& password,
			       unsigned int index = 0);
  };

}

#endif
