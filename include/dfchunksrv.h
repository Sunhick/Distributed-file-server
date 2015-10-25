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
#include <vector>

#include "dfcomm.h"
#include "dfconfig.h"

namespace dfs {
  class df_chunk_srv {
  private:
    std::string server_id;
    // file system for the server
    std::string filesys = "/DFS#";
    int port = -1;
    int sockfd;
    generic_comm* communication;
    dfconfig* config;

    // lock for maintaining the consistancy on pending requests
    std::mutex request_lock;
    // list of pending requests
    std::map<std::thread::id, std::thread&&> pending_requests;

    // handle the df client server request
    void dispatch_request(int newfd);

    // switch to the user session. 
    void switch_session(std::string user);

    // get the list of all files given the directory
    void get_all_files(std::vector<std::string> &out, const std::string &directory);
    void handle(std::string request, int sockfd);

  public:
    df_chunk_srv(std::string filesys, int port);
    ~df_chunk_srv();
    
    void start();
    void listen_forever();
    // list all available files under the server
    void list(int newfd, std::string& args, const std::string& username);
    // get the piece information from the server
    void get(int newfd, std::string& args, const std::string& username);
    // store the chunk file in server
    void put(int newfd, std::string& content, const std::string& username);
    // make directory for the user
    void mkdir(int newfd, std::string& arguments, const std::string& username);
  };
}

#endif
