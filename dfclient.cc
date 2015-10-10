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
#include <string.h>

#include <vector>
#include <thread>
#include <chrono>
#include <fstream>
#include <iostream>

#include "include/dfutils.h"

using namespace dfs;

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

void df_client::list()
{
  system("clear");
  request->set_command("LIST");

  int ii = 0;
  char buff[2048*2] = {'\0'};
  
  std::string command = request->to_string();
  std::cout << "listing contents" << std::endl;
  
  for (auto& server : this->channels) {
    server.second->write(this->sockfds[ii], command.c_str(), command.size());
    if (server.second->read(this->sockfds[ii++], buff, 2048*2) == 0) {
      std::cout << "unable to read the data" << std::endl;
      continue;
    }

    std::string data(buff);

    df_reply_proto reply(data);
    if (reply.ecode == ERR_INVALID_USER) {
      std::cout << "ERROR: " << reply.emsg << std::endl;
      continue; //continue processing with other users
    }

    // split based on delimiter
    auto files = utilities::split(reply.contents,
				  [](int ch){ return (ch == ' '? 1 : 0); });
    for (auto file : files)
      std::cout << file << " [status]" <<"\n";
  }
}

void df_client::get() 
{
  system("clear");
  std::cout << "Enter file name to get:";
  std::string file("");
  std::cin >> file;
  request->set_command("GET", file);
  std::string cmd = request->to_string();

  int ii = 0;
  for (auto& channel : this->channels) {
    auto& server = channel.second;
    server->write(this->sockfds[ii], cmd.c_str(), cmd.size());

    // while(true) {
    char buff[2048*2] = {'\0'};
    if (server->read(this->sockfds[ii], buff, 2048*2) == 0) {
      std::cout << "unable to read the data" << std::endl;
      continue;
      // continue;
    }

    std::string reply = std::string(buff);
    std::cout << reply << std::endl;
    // df_reply_proto response(reply);
    //}
    ii++;
    // reproduce the file from the server reply
  } // for
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

void df_client::put()
{
  system("clear");
  std::cout << "Enter the file to put:";
  std::string filename("");
  std::cin >> filename;
  // request->arguments = std::string("");

  // based on the policy number, place the chunk file
  // in different chunk servers
  int policy = get_policy(filename);
  std::cout << "policy : " << policy << std::endl;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cout << "Error in reading file!" << filename << "\n";
    return;
  }
  /*
    file.seekg(0, std::ios_base::end);
    size_t size = file.tellg();
    file.seekg(0);
  */
  // file.seekg(0, std::ios::end);
  file.seekg(0, std::ios_base::end);
  size_t size = file.tellg();
  std::string buffer(size, ' ');
  file.seekg(0);
  file.read(&buffer[0], size);

  /*  const size_t blockSize = 2048*5;
      std::streampos pos = 0;
      std::string data;*/
 
  // write the actual file content in the df_reply_proto format
  // while ((pos = file.tellg()) >= 0 && (size_t)pos < size - 1){ 
  // data.resize(size - (size_t)pos < blockSize ? size - (size_t)pos : blockSize);
  //file.read(&data[0], data.size());

  // send all the contents
  request->set_command("PUT", filename + ":" + buffer);
  std::string cmd = request->to_string();
    
  // std::this_thread::sleep_for (std::chrono::seconds(1));
  /*    char ch;
	std::cin >> ch;
	system("clear");*/
  std::cout << "PUT CMD:" << cmd << std::endl;
  //todo: send data only to selected servers
  int ii = 0;
  for (auto& server : this->channels)
    server.second->write(this->sockfds[ii++], cmd.c_str(), cmd.length());

  // std::this_thread::sleep_for (std::chrono::seconds(1));
  //}
  
  file.close();
}

void df_client::start()
{
  // connect to all running servers
  for (auto& server : this->channels) {
    auto channel = server.second;
    auto id = channel->connect();
    sockfds.push_back(id);
  }

  while(true) {
    int choice;
    std::cout << "Enter your choice:" << "1. LIST  2. GET  3. PUT  4.EXIT" << std::endl;
    std::cin >> choice;
    
    switch(choice) {
    case 1:
      {
	list();
	break;
      }
    case 2:
      {
	get();
	break;
      }
    case 3:
      {
	put();
	break;
      }

    default:
    case 4:
      {
	for (int id : sockfds)
	  close(id);
	exit(EXIT_SUCCESS);
	break;
      }      
    } // switch
  } // while
}

int main(int argc, char *argv[])
{
  std::cout << "DF Client"  << "\n";

  std::string file(argv[1]);
  std::cout << file << std::endl;

  df_client client(file);
  client.start();

  return 0;
}
