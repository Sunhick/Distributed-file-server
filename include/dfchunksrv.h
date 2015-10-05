/****************************************************
 *  Distributed file chunk server header file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_CHUNK_SRV_H
#define DFS_CHUNK_SRV_H

#include <string>
#include <map>
#include <thread>
#include <mutex>

namespace dfs {
  class df_chunk_srv {
  private:
    std::string server_id;
    // file system for the server
    std::string filesys = "/DFS#";
    int port = -1;
    int sockfd;

    // lock for maintaining the consistancy on pending requests
    std::mutex request_lock;
    // list of pending requests
    std::map<std::thread::id, std::thread&&> pending_requests;

    // handle the df client server request
    void dispatch_request(int newfd);
    int die(const char *format, ...);

  public:
    df_chunk_srv(std::string filesys, int port);
    ~df_chunk_srv();
    
    void start();
    int open_socket(int backlog);
    void listen_forever();
    // list all available files under the server
    void list();
    // get the piece information from the server
    void get(std::string filename);
    // store the chunk file in server
    void put(std::string filename, std::string content);
  };
}

#endif
