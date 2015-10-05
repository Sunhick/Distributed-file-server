/****************************************************
 *  Distributed file chunk server implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfchunksrv.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <exception>
#include <csignal>
#include <thread>
#include <iostream>

#define PERMISSIONS 0777
#define BACKLOG 32

int g_sock = -1;

using namespace dfs;

df_chunk_srv::df_chunk_srv(std::string filesys, int port) 
{
  if (filesys.empty())
    throw std::string("File system is not mentioned");  

  this->communication = new df_socket_comm("localhost", port);
  
  this->port = port;
  this->filesys = filesys;
  struct stat st = {0};
  // create a File system if it doesn't exists
  if (stat(filesys.c_str(), &st) == -1) {
    mkdir(filesys.c_str(), PERMISSIONS);
  }
}

df_chunk_srv::~df_chunk_srv() 
{
  
}

void df_chunk_srv::start()
{
  try {
    // open master socket for client to connect
    // a maximum of 32 client can connect simultaneously.
    g_sock = sockfd = this->communication->open(BACKLOG); // open_socket(BACKLOG);

    // unable to open socket.
    if (sockfd < 0)
      return;

    // Install a signal handler for graceful shut of server
    std::signal(SIGINT, [](int signal) {
	std::cout << "\nSignal caught!" << "Closing file server socket..." << std::endl;
	close(g_sock);
	exit(EXIT_FAILURE);
      });
    
    // Accept incoming connections & launch
    // the request thread to service the clients
    while (true) {
      int newfd = communication->accept(sockfd);
      if (newfd == -1) {
	perror("accept");
	continue;
      }

      // fork vs thread ?
      // Fork : overhead of creating process
      // thread : lightweight process and faster in setting up
      // provide service to client in a different thread

      // create thread t on heap. creating t on stack leads to terminate exception,
      // as t is scoped(stack) variable. 
      std::thread *t = new std::thread(&df_chunk_srv::dispatch_request, this, newfd);
      auto id = t->get_id();
      std::cout << id << std::endl;
      auto entry = std::pair<std::thread::id, std::thread&&>(id, std::move(*t));

      std::lock_guard<std::mutex> lock(request_lock);
      pending_requests.insert(std::move(entry));

      std::cout << "Accepted the client! Client Count:" << pending_requests.size() << std::endl;
    }

    // Make sure all clients are serviced, before server goes down
    for (auto &thread : pending_requests)
      thread.second.join();
    
  } catch (std::exception& e) {
    std::cout << "Error in server(main) thread! Reason: " << e.what() << std::endl;
  } catch (...) {
    std::cout << "Error in server(main) thread!" << std::endl;
  }
}

void df_chunk_srv::dispatch_request(int newfd)
{
  std::cout << "Worker thread(client):"  << std::this_thread::get_id()
	    <<  " socket Id: " << newfd << std::endl;

  while (true) {
    char buff[4096];      
    if (communication->read(newfd, buff, 4096) == 0) {
      std::cout << "Unable to read the socket!" << std::endl;
      break;
    }

    // process the buffer if it contains request
    if (buff == NULL || strlen(buff) == 0) break;

    std::string reqlines = std::string(buff);
    std::cout << "Request:" << reqlines << std::endl;
  }

  // close the socket
  close(newfd);

  // release from the pending_requests
  auto id = std::this_thread::get_id();
  if (this->pending_requests.find(id) != this->pending_requests.end()) {
    std::lock_guard<std::mutex> lock(this->request_lock);
    // figure out how to delete thread t
    // std::thread&& t = std::move(this->pending_requests[id]);
    // delete t;
    this->pending_requests.erase(id);
  }
}


void df_chunk_srv::listen_forever()
{
  this->start();
}

void df_chunk_srv::list()
{
  
}

void df_chunk_srv::get(std::string filename)
{
  
}

void df_chunk_srv::put(std::string filename, std::string content)
{

}
