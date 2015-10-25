/****************************************************
 *  df communication abstraction header file. 
 *  
 *  This file hides the underlying communication
 *  mechanism and provides a uniform communication
 *  between two hosts. communication be pipe, socket, 
 *  file etc.
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_COMM_H
#define DFS_COMM_H

#include <string>

namespace dfs {

  // Abstract the communication between two same/different hosts
  // generic communication interface
  class generic_comm {
  protected:
    int socketfd;
    std::string host;
    int port;

  public:
    // open the communication channel
    virtual int open(int backlog) = 0;
    
    // accept the incoming connections
    virtual int accept(int sockfd) = 0;

    // connect to the host channel
    virtual int connect() = 0;

    // read the data from host
    virtual ssize_t read(int fd, void *buf, size_t count) = 0;
    virtual ssize_t read(void* buf, size_t count) = 0;

    // write data to the host
    virtual ssize_t write(int fd, const void *buf, size_t count) = 0;
    virtual ssize_t write(const void *buf, size_t count) = 0;
    
    generic_comm() { } 
    virtual ~generic_comm() { }
  };

  // defines the socket based communication between client and server
  class df_socket_comm : public generic_comm {
  private:
    int die(const char *format, ...);

  public:
    df_socket_comm(std::string host, int port);
    ~df_socket_comm();
    
    virtual int open(int backlog);
    virtual int accept(int sockfd);
    virtual int connect();

    virtual ssize_t write(int fd, const void *buf, size_t count);
    virtual ssize_t write(const void *buf, size_t count);

    virtual ssize_t read(int fd, void *buf, size_t count);
    virtual ssize_t read(void *buf, size_t count);
  };

  // defines the pipe based communication between two process
  class df_pipe_comm : public generic_comm {
    // TODO: Implement generic_comm interface
  };

  // defines the memory mapped based communication
  class df_memory_mapped_comm : public generic_comm {
    // TODO: memory mapped comm.
  };

  // defines the shared memory based communication
  class df_shared_memory_comm : public generic_comm {
    // TODO: shared memory communication
  };
}

#endif
