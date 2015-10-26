/****************************************************
 *  df client implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfclient.h"

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/md5.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>

#include <string>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>

#include "include/dfutils.h"

using namespace dfs;

modifier green(color::FG_GREEN);
modifier def(color::FG_DEFAULT);
modifier red(color::FG_RED);

df_client::df_client(std::string& file)
{
  this->config = new dfconfig(file);
  std::string name, password;
  this->config->get_username_password(name, password);
  this->request = new df_request_proto(name, password);

  init_upload_policies();
  
  for (auto server : this->config->get_all_servers()) {
    auto ip_address = server.second.ip_address;
    auto port = server.second.port;
    auto comm = new df_socket_comm(ip_address, port);
    if (comm)
      this->channels.insert(std::pair<std::string, df_socket_comm*>(server.first, comm));
  }
}

df_client::~df_client()
{
  delete this->config;
  delete this->request;
}

void df_client::init_upload_policies()
{
  std::vector<struct upload_policy> p0, p1, p2, p3;

  //----------- x = 0
  struct upload_policy p;
  p.name = "DFS1";
  p.chunk_ids.push_back(1);
  p.chunk_ids.push_back(2);
  p0.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS2";
  p.chunk_ids.push_back(2);
  p.chunk_ids.push_back(3);
  p0.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS3";
  p.chunk_ids.push_back(3);
  p.chunk_ids.push_back(4);
  p0.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS4";
  p.chunk_ids.push_back(4);
  p.chunk_ids.push_back(1);
  p0.push_back(p);
  
  //----------- x = 1
  p.chunk_ids.clear();
  p.name = "DFS1";
  p.chunk_ids.push_back(4);
  p.chunk_ids.push_back(1);
  p1.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS2";
  p.chunk_ids.push_back(1);
  p.chunk_ids.push_back(2);
  p1.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS3";
  p.chunk_ids.push_back(2);
  p.chunk_ids.push_back(3);
  p1.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS4";
  p.chunk_ids.push_back(3);
  p.chunk_ids.push_back(4);
  p1.push_back(p);

  //----------- x = 2
  p.chunk_ids.clear();
  p.name = "DFS1";
  p.chunk_ids.push_back(3);
  p.chunk_ids.push_back(4);
  p2.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS2";
  p.chunk_ids.push_back(4);
  p.chunk_ids.push_back(1);
  p2.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS3";
  p.chunk_ids.push_back(1);
  p.chunk_ids.push_back(2);
  p2.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS4";
  p.chunk_ids.push_back(2);
  p.chunk_ids.push_back(3);
  p2.push_back(p);

  //----------- x = 3
  p.chunk_ids.clear();
  p.name = "DFS1";
  p.chunk_ids.push_back(2);
  p.chunk_ids.push_back(3);
  p3.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS2";
  p.chunk_ids.push_back(3);
  p.chunk_ids.push_back(4);
  p3.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS3";
  p.chunk_ids.push_back(4);
  p.chunk_ids.push_back(1);
  p3.push_back(p);

  p.chunk_ids.clear();
  p.name = "DFS4";
  p.chunk_ids.push_back(1);
  p.chunk_ids.push_back(2);
  p3.push_back(p);  

  upload_policies.insert(std::pair<int, std::vector<struct upload_policy>>(0, p0));
  upload_policies.insert(std::pair<int, std::vector<struct upload_policy>>(1, p1));
  upload_policies.insert(std::pair<int, std::vector<struct upload_policy>>(2, p2));
  upload_policies.insert(std::pair<int, std::vector<struct upload_policy>>(3, p3));  
}

bool df_client::server_timeout(int filedesc) 
{
  fd_set set;
  struct timeval timeout;

  FD_ZERO(&set); // clear the set
  FD_SET(filedesc, &set); // add our file descriptor to the set 

  timeout.tv_sec = 1;
  int rv = select(filedesc + 1, &set, NULL, NULL, &timeout);

  if(rv == -1) {
    std::cout << "Error in select()" << std::endl;
    return true;
  } else if(rv == 0) {
    std::cout << "server response time out!" << std::endl;
    return true;
  }

  return false;
}

void df_client::list(std::string folder)
{
  request->set_command("LIST");
  request->arguments = folder;
  
  std::string command = request->to_string();

  // map of file names and chucks
  std::map<std::string, std::vector<int>> files;

  for (auto& server : this->channels) {
    char buff[2048*2] = {'\0'};

    server.second->write(command.c_str(), command.size());

    if (server_timeout(server.second->get_file_descriptor())) {
      std::cout << "server response time out!" << std::endl;
      continue;  // process other servers
    }

    if (server.second->read(buff, 2048*4) == 0) {
      this->channels.erase(server.first);
      std::cout << "unable to read the data. Server is down! Updated the server list!" << std::endl;
      std::cout << "No. of alive servers: " << this->channels.size() << std::endl;
      continue;
    }

    std::string data(buff);

    df_reply_proto reply(data);
    if (reply.ecode == ERR_INVALID_USER) {
      std::cout << "ERROR: " << reply.emsg << std::endl;
      continue; //continue processing with other users
    }

    // std::cout << server.first << " raw-response: " + reply.contents << std::endl;
    // split based on delimiter
    auto filenames = utilities::split(reply.contents,
				      [](int ch){ return (ch == ' '? 1 : 0); });

    for (auto &name : filenames) {
      auto tilldot = name.find_last_of(".");
      if (tilldot == std::string::npos) {
	std::cout << "no extension found! " << name << std::endl;
	continue;
      }

      auto file = name.substr(0, tilldot);
      auto chunk = std::atoi(name.substr(tilldot+1).c_str());
      if (files.find(file) == files.end())
       	files.insert(std::pair<std::string, std::vector<int>>(file, std::vector<int>()));

      auto chunks = files[file];

      if (std::find(chunks.begin(), chunks.end(), chunk) == chunks.end())
	files[file].push_back(chunk);
    }
  }

  auto get_file_status  = [](int fcount) { return fcount == 4 ? "Complete" : "Incomplete"; };

  for (auto &name : files) {
    auto status = get_file_status(name.second.size());
    auto status_color = status == std::string("Complete") ? green : red;
    std::cout << std::left << std::setw(20) << name.first << std::right << std::setw(10) << "\t[ " <<
      status_color << status << def << " ]" << std::endl;    
  }
}

void df_client::get(std::string file) 
{
  request->set_command("GET", file);
  std::string cmd = request->to_string();

  std::map<int, std::string> part_files;
  bool skip_next = false;

  for (auto& channel : this->channels) {
    // reduce traffic on the network by not getting the redunant files from
    // from the next server. but instead try the next to next server
    // if (skip_next)  {
    //   skip_next = false;
    //   continue;
    // }

    auto& server = channel.second;
    if (server->write(cmd.c_str(), cmd.size()) < 0) {
      std::cout << "error in writing to the socket. Maybe server is down" << std::endl;
      this->channels.erase(channel.first);
      continue;
    }

    if (server_timeout(server->get_file_descriptor())) {
      std::cout << "Server time out " << std::endl;
      continue;
    }

    for (int j = 0 ; j < 2; j++) {
      char buff[2048*2] = {'\0'};
      if (server->read(buff, 2048*4) == 0) {
	std::cout << "unable to read the data. server maybe down!" << std::endl;
	this->channels.erase(channel.first);
	break;
      }

      df_reply_proto reply(buff);
      if (reply.ecode != OK) {
	std::cout << "ERROR: " << reply.emsg << std::endl;
	break; //continue processing with other users
      }

      std::string name;
      int chunk;
      if (!get_filename_chunknum(reply.emsg, name, chunk)) continue;

      if (part_files.find(chunk) == part_files.end()) {
	part_files.insert(std::pair<int, std::string>(chunk, reply.contents));
	// skip_next = true;
      } else {
	// skip_next = false;
      }
    } // for 2 times
  } // for each server

  if (part_files.size() < 4) {
    std::cout << red << "Missing files! Unable to assemble file" << def << std::endl;
    return;
  }
  
  std::string fpath = "download.txt";
  std::fstream dfile(fpath, std::ofstream::out | std::ofstream::in);
  if (dfile.is_open()) {
    // warning! file already exists. Maybe the client wants to append
    // to existing file.
    dfile.seekg(0, std::ios::end);
  } else {
    // create the file
    dfile.clear();
    dfile.open(fpath, std::ofstream::out);
  }

  dfile << "--------------------" << file << "--------------------------" << std::endl;
  
  for (auto& file : part_files)
    dfile << file.second;

  std::cout << green << "File downloaded! check download.txt" << def <<std::endl;
  dfile.close();
}

bool df_client::get_filename_chunknum(const std::string& msg,
				      std::string& filename,
				      int& chunk) const
{
  auto tilldot = msg.find_last_of(".");
  if (tilldot == std::string::npos) {
    std::cout << red << "No extension found! " << msg << def << std::endl;
    return false;
  }

  filename = msg.substr(0, tilldot);
  chunk = std::atoi(msg.substr(tilldot+1).c_str());
  return true;
}

// Get the number that represents the policy as to where to
// put pieces of file in which servers
// policy = MD5HASH(filename) % 4
int df_client::get_policy(std::string file)
{
  MD5_CTX c;
  unsigned char out[MD5_DIGEST_LENGTH];

  MD5_Init(&c);
  MD5_Update(&c, file.c_str(), file.length());
  MD5_Final(out, &c);

  std::string md5;

  char buf[MD5_DIGEST_LENGTH];
  for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
    sprintf(buf, "%02x", out[i]);
    md5.append( buf );
  }

  // to compute MD5HASH(file) % 4, we need only last byte
  short num = md5[MD5_DIGEST_LENGTH];
  // md5[MD5_DIGEST_LENGTH] | (md5[MD5_DIGEST_LENGTH - 1] << 8);
  return num % 4;
}

void df_client::put(std::string filename, std::string folder)
{
  // based on the policy number, place the chunk file
  // in different chunk servers
  int policy = get_policy(filename);

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cout << red << "Error in reading file:" << filename << def <<"\n";
    return;
  }

  file.seekg(0, std::ios_base::end);
  size_t size = file.tellg();
  std::string buffer(size, ' ');
  file.seekg(0);
  file.read(&buffer[0], size);

  int part_size = (int)size / this->channels.size();

  std::vector<std::string> parts;
  parts.push_back(buffer.substr(0*part_size, 1*part_size));
  parts.push_back(buffer.substr(1*part_size, 2*part_size));
  parts.push_back(buffer.substr(2*part_size, 3*part_size));
  parts.push_back(buffer.substr(3*part_size, size - 3*part_size));

  for (auto &polices : upload_policies[policy]) {
    for (auto &chunk_num : polices.chunk_ids) {
      request->set_command("PUT",
			   filename + "." + std::to_string(chunk_num) + ":" + parts[chunk_num-1]);
      std::string cmd = request->to_string();
      this->channels[polices.name]->write(cmd.c_str(), cmd.length());
    }
  }

  file.close();
}

void df_client::mkdir(std::string dirname)
{
  request->set_command("MKDIR");
  request->arguments = std::string(dirname);

  for (auto &channel : this->channels) {
    auto cmd = request->to_string();
    channel.second->write(cmd.c_str(), cmd.length());
  }
}

void df_client::start()
{
  // connect to all running servers
  for (auto& server : this->channels) {
    auto channel = server.second;
    auto id = channel->connect();
    sockfds.push_back(id);
  }

  auto cmd_to_int = [](std::string cmd) {
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), (int(*)(int))std::tolower);
    if (cmd == "list") return 1;
    if (cmd == "get") return 2;
    if (cmd == "put") return 3;
    if (cmd == "mkdir") return 4;
    if (cmd == "exit") return 5;
    return -1;
  };

  std::cout << ".........................................\n"
	    << "Distributed file server v1.0\n"
	    << "Commands: LIST, GET, PUT, MKDIR, EXIT & HELP\n"
	    << ".........................................." << std::endl;

  while(true) {
    std::string cmd("");
    std::cout << "dfs>>> " << std::flush;
    std::getline(std::cin, cmd);
    if (cmd.empty()) continue;
    auto args = utilities::split(cmd, [](int c){ return c == ' ' ? 1 : 0; });
    
    switch(cmd_to_int(args[0])) {
    case 1:
      {
	std::string folder("");
	if (args.size() > 1) {
	  folder = args[1];
	}

	list(folder);
	break;
      }
    case 2:
      {
	if (args.size() < 2) {
	  std::cout <<red << "Invalid parameters!" << std::endl << def;
	  continue;
	}

	get(args[1]);
	break;
      }
    case 3:
      {
	if (args.size() < 2) {
	  std::cout << red << "Invalid parameters!" << std::endl << def;
	  continue;
	}
	std::string folder;
	if (args.size() > 2) {
	  folder = args[2];
	}

	put(args[1], folder);
	break;
      }
    case 4:
      {
	if (args.size() < 2) {
	  std::cout << red << "Invalid parameters!" << std::endl << def;
	  continue;
	}

	mkdir(args[1]);
	break;
      }
    case 5:
      {
	for (int id : sockfds)
	  close(id);
	exit(EXIT_SUCCESS);
	break;
      }
    default:
      {
	std::cout << "Unknown command! Please try again!" << std::endl;
	break;
      }
    } // switch
  } // while
}

int main(int argc, char *argv[])
{
  system("clear");
  std::string file(argv[1]);
  
  df_client client(file);
  client.start();

  return 0;
}
