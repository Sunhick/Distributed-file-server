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
#include <chrono>
#include <iostream>
#include <fstream>
#include <regex>

#include "include/dfproto.h"
#include "include/dfutils.h"

#define PERMISSIONS 0777
#define BACKLOG 32

int g_sock = -1;

using namespace dfs;

df_chunk_srv::df_chunk_srv(std::string filesys, int port) 
{
  if (filesys.empty())
    throw std::string("File system is not mentioned");  

  this->communication = new df_socket_comm("localhost", port);
  this->config = new dfconfig("dfs.conf");
  
  this->port = port;
  this->filesys = filesys;
  struct stat st = {0};
  // create a File system if it doesn't exists
  if (stat(filesys.c_str(), &st) == -1) {
    ::mkdir(filesys.c_str(), PERMISSIONS);
  }
}

df_chunk_srv::~df_chunk_srv() 
{
  delete this->communication;
  delete this->config;
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

    char buff[2048*2] = {'\0'};
    if (communication->read(newfd, buff, 2048*4) < 0) {
      std::cout << "Client closed! Unable to read the socket!" << std::endl;
      break;
    }

    // process the buffer if it contains request
    if (buff == NULL || strlen(buff) == 0) break;
    std::string request = std::string(buff);
    // std::cout << "raw-request:" << request << std::endl;
    std::cout << "Raw-request: "
	      << std::string(request.begin(), request.begin() + 30)
	      << "..."<< std::endl;
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

void df_chunk_srv::switch_session(std::string username)
{
  std::string filepath = this->filesys + "/" + username;
  struct stat st = {0};
  // create a File system if it doesn't exists
  if (stat(filepath.c_str(), &st) == -1) {
    ::mkdir(filepath.c_str(), PERMISSIONS);
  }
}

// entry point of all client request handling.
// first validate the user and then process the request
void df_chunk_srv::handle(std::string reqstr, int newfd) 
{
  df_request_proto request(reqstr);

  // validate the user and only then service the request.
  if (!this->config->validate(request.username, request.password)) {
    std::string error("Invalid username/ password. Please try again");
    df_reply_proto reply(ERR_INVALID_USER, error, " ");
    std::string rstr = reply.to_string();
    this->communication->write(newfd, rstr.c_str(), rstr.size());
    return;
  }

  //look for the user directory. Create one if not available
  std::string username = request.username;
  switch_session(username);
  
  std::string cmd = request.command;
  std::string args = request.arguments;
  
  if (cmd == "LIST") {
    list(newfd,args, username);
  } else if (cmd == "GET") {
    get(newfd, args, username);
  } else if (cmd == "PUT") {
    put(newfd, args, username);
  } else if (cmd == "MKDIR") {
    mkdir(newfd, args, username);
  } else {
    std::cout << "Invalid request from df client! cmd:" << cmd << std::endl;
  }
}

void df_chunk_srv::listen_forever()
{
  this->start();
}

void df_chunk_srv::mkdir(int newfd, std::string& arguments,
			 const std::string& username)
{
  std::string folder = filesys + "/" + username  + "/" + arguments;
  std::cout << "MKDIR : " << folder << std::endl;
  struct stat st = {0};
  // create a File system if it doesn't exists
  if (stat(folder.c_str(), &st) == -1) {
    ::mkdir(folder.c_str(), PERMISSIONS);
  }
}

void df_chunk_srv::list(int newfd,
			std::string& arguments,
			const std::string& username)
{
  std::vector<std::string> files;
  std::string dir = this->filesys + "/" + username;
  std::cout << "list: " << dir << std::endl;
  this->get_all_files(files, dir);
  std::cout << "Get all files size: " << files.size() << std::endl;

  std::string data(" ");
  if (files.size()) {
    for (auto& file : files)
      data += file + " ";
    std::cout << "List of files:" << data << std::endl;
  }

  df_reply_proto reply(OK, "LIST", data);
  std::string rstr = reply.to_string();
  this->communication->write(newfd, rstr.c_str(), rstr.size());
}

void df_chunk_srv::get(int newfd,
		       std::string& filename,
		       const std::string& username)
{
  auto file_exists = [] (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);    
  };

  std::regex re(std::string(filename + "\\.[0-9]+"));

  std::vector<std::string> allfilenames;
  this->get_all_files(allfilenames, this->filesys + "/" + username);

  int matches = 0;
  
  for (auto& filename : allfilenames) {
    if (!std::regex_match(filename, re)) continue;

    matches++;

    std::string filepath = this->filesys + "/" + username + "/." + filename;
    std::ifstream file(filepath);
    if (!file.is_open()) {
      std::cout << "Error in reading file!" << filepath << "\n";
      df_reply_proto reply(ERR_CORRUPTED_FILE, "unable to read!", " ");
      std::string rstr = reply.to_string();
      this->communication->write(newfd, rstr.c_str(), rstr.size());
      return;
    }

    file.seekg(0, std::ios_base::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);
  
    df_reply_proto reply(OK, filename, buffer);
    std::string rstr = reply.to_string();
    this->communication->write(newfd, rstr.c_str(), rstr.size());
    file.close();     
  }

  // no matches found, inform the client about no files found.
  if (!matches) {
    std::string filepath = this->filesys + "/" + username + "/." + filename;
    if(!file_exists(filepath)) {
      std::cout << "File not found: ["  << filepath << "]" << std::endl;
      df_reply_proto reply(ERR_FILE_NOT_FOUND, "File doesn't exists!", " ");
      std::string rstr = reply.to_string();
      this->communication->write(newfd, rstr.c_str(), rstr.size());
      return;
    }
  }
}

void df_chunk_srv::put(int newfd,
		       std::string& content,
		       const std::string& username)
{
  // split the content to extract the filename
  std::string::size_type pos;
  pos = content.find(':', 0);

  if (pos != std::string::npos) {
    std::string filename = content.substr(0, pos); 
    std::string data = content.substr(pos+1, content.size());

    std::string fpath = this->filesys + "/" + username + "/." + filename;
    std::fstream file(fpath, std::ofstream::out | std::ofstream::in);
    if (file.is_open()) {
      // warning! file already exists. Maybe the client wants to append
      // to existing file.
      file.seekg(0, std::ios::end);
    } else {
      // create the file
      file.clear();
      file.open(fpath, std::ofstream::out);
    }

    file << data;
    file.close();    
  }
}

void df_chunk_srv::get_all_files(std::vector<std::string> &out,
				 const std::string &directory)
{
  DIR *dir;
  class dirent *ent;
  class stat st;

  dir = opendir(directory.c_str());
  while ((ent = readdir(dir)) != NULL) {
    const std::string file_name = ent->d_name;
    const std::string full_file_name = directory + "/" + file_name;

    if (stat(full_file_name.c_str(), &st) == -1)
      continue;

    const bool is_directory = (st.st_mode & S_IFDIR) != 0;

    if (is_directory)
      continue;

    // get rid of dot prefix for the files
    out.push_back(file_name.substr(1));
  }
  closedir(dir);
} 
