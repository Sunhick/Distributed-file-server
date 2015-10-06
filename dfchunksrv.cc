/****************************************************
 *  Distributed file chunk server implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfchunksrv.h"

#include <sys/stat.h>

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <dirent.h>

#include <sstream>
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
    g_sock = sockfd = this->communication->open(BACKLOG);

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
    char buff[4096] = {'\0'};
    if (communication->read(newfd, buff, 4096) == 0) {
      std::cout << "Unable to read the socket!" << std::endl;
      break;
    }

    // process the buffer if it contains request
    if (buff == NULL || strlen(buff) == 0) break;

    std::string request = std::string(buff);
    std::cout << "raw request:" << request << std::endl;
    handle(request, newfd);
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

void df_chunk_srv::handle(std::string request, int newfd) {
  std::stringstream parse(request);
  std::string str;

  parse >> str;

  std::cout << "request : " << str << std::endl;
  
  if (str == "LIST") {
    std::cout << "In list " << std::endl;
    list(newfd);
  } else if (str == "GET") {
    std::string filename;
    get(filename);
  } else if (str == "PUT") {
    std::string filename, content;
    put(filename, content);
  } else {
    std::cout << "Invalid request from df client!" << std::endl;
  }
}

void df_chunk_srv::listen_forever()
{
  this->start();
}

void df_chunk_srv::list(int newfd)
{
  std::vector<std::string> files;
  this->get_all_files(files, this->filesys);
  std::cout << "Get all files size: " << files.size() << std::endl;

  if (files.size()) {
    std::string data;
    for (auto file : files) {
      data += file + " ";
      std::cout << data << std::endl;
    }
    data += '\0';  // null terminated string
    std::cout << "List of files:" << data << std::endl;
    this->communication->write(newfd, data.c_str(), data.size());
  }
}

void df_chunk_srv::get(std::string filename)
{
  
}

void df_chunk_srv::put(std::string filename, std::string content)
{

}


void df_chunk_srv::get_all_files(std::vector<std::string> &out, const std::string &directory)
{
  DIR *dir;
  class dirent *ent;
  class stat st;

  dir = opendir(directory.c_str());
  while ((ent = readdir(dir)) != NULL) {
    const std::string file_name = ent->d_name;
    const std::string full_file_name = directory + "/" + file_name;

    if (file_name[0] == '.')
      continue;

    if (stat(full_file_name.c_str(), &st) == -1)
      continue;

    const bool is_directory = (st.st_mode & S_IFDIR) != 0;

    if (is_directory)
      continue;

    out.push_back(file_name);
  }
  closedir(dir);
} 
