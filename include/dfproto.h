/****************************************************
 *  Distributed file communication protocol
 *  implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_PROTO_H
#define DFS_PROTO_H

#define OK			100

#define ERR_INVALID_USER	101	// Invalid user name 
#define ERR_INVALID_PWD		102	// Invalid password
#define ERR_SERVER_CRASH	103	// server crashed while processing request but recovering
#define ERR_CORRUPTED_FILE	104	// file corrupted at server
#define ERR_NO_FILES		105	// no files found for selected user
#define ERR_FILE_NOT_FOUND	106	// files couldn't be found in the user directory

#define ERR_GENERAL		500	// general error, may be a bug. look at log file

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
  public:
    std::string username;
    std::string password;
    std::string command;
    std::string arguments;

    df_request_proto(const std::string& name,
		     const std::string& password);
    // take a request string and convert into
    // df_request_proto object
    df_request_proto(const std::string& request);
    ~df_request_proto();
    void set_command(const std::string& command,
		     const std::string& args = "");
    std::string to_string();
  };

  /*
   * +-----------------------------+
   * | Status code |  Message      |
   * +-----------------------------+
   * | headers                     |
   * +-----------------------------+
   * | Contents (optional)         |
   * +-----------------------------+
   * 
   */
  class df_reply_proto {
  public:
    int ecode;
    std::string emsg;
    std::string contents;

    // takes a reply string and constructs a df_reply_proto
    // object for easier handling
    df_reply_proto(std::string reply);
    df_reply_proto(int ecode, std::string emsg, std::string contents);
    ~df_reply_proto();

    std::string to_string();
  };
}

#endif
