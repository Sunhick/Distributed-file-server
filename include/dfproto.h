/****************************************************
 *  Distributed file communication protocol
 *  implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_PROTO_H
#define DFS_PROTO_H

#include <string>

namespace dfs {
  /*
   *  +---------------------------+
   *  |  user name   |  password  |
   *  +---------------------------+
   *  | command      |  arguments |
   *  +---------------------------+
   *
   *  valid commands: LIST, PUT, GET
   */
  class df_request_proto {
  private:
    std::string username;
    std::string password;
    std::string command;
    std::string arguments;
    
  public:
    df_request_proto(const std::string& name,
		     const std::string& password);
    ~df_request_proto();
    void set_command(const std::string& command,
		     const std::string& args = "");
    std::string to_string();
  };

  class df_reply_proto {
    
  };
}

#endif
