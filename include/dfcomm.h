/****************************************************
 *  df communication abstraction header file. 
 *  
 *  This file hides the underlaying communication
 *  mechanism and provides a uniform access. It can
 *  pipe, socket, file etc.
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_COMM_H
#define DFS_COMM_H

#include <string>

namespace dfs {
  // generic communication interface
  class generic_comm {
  public:
    virtual int open(int backlog) = 0;
    virtual int accept(int sockfd) = 0;
    virtual int connect() = 0;
    virtual ssize_t read(int fd, void *buf, size_t count) = 0;
    virtual ssize_t write(int fd, const void *buf, size_t count) = 0;
    
    generic_comm() { } 
    virtual ~generic_comm() { }
  };

  // defines the socket communication between client and server
  class df_socket_comm : public generic_comm {
  private:
    int socketfd;
    std::string host;
    int port;

    int die(const char *format, ...);

  public:
    df_socket_comm(std::string host, int port);
    ~df_socket_comm();
    
    virtual int open(int backlog);
    virtual int accept(int sockfd);
    virtual int connect();

    virtual ssize_t write(int fd, const void *buf, size_t count);
    virtual ssize_t read(int fd, void *buf, size_t count);
  };
}

#endif
