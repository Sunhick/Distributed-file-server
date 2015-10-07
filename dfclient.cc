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

#include <iostream>
#include <string.h>

#include "include/dfutils.h"

using namespace dfs;

df_client::df_client(std::string& file)
{
  this->config = new dfconfig(file);
  std::string name, password;
  this->config->get_username_password(name, password);
  this->request = new df_request_proto(name, password);

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

}

void df_client::list()
{
  system("clear");
  request->set_command("LIST");

  int ii = 0;
  char buff[2048] = {'\0'};
  
  std::string command = request->to_string();
  std::cout << "listing contents" << std::endl;
  
  for (auto& server : this->channels) {
    server.second->write(this->sockfds[ii], command.c_str(), command.size());
    if (server.second->read(this->sockfds[ii++], buff, 2048) == 0) {
      std::cout << "unable to read the data" << std::endl;
      continue;
    }

    std::string data(buff);

    // split based on delimiter
    auto files = utilities::split(data, [](int ch){ return (ch == ' '? 1 : 0); });
    for (auto file : files)
      std::cout << file << " [complete]" <<"\n";
  }
}

void df_client::get() 
{
  system("clear");
  std::cout << "Enter file name to get:";
  std::string file("");
  std::cin >> file;
  char buf[128] = "GET hello.txt\0";
  auto outchars = strlen(buf);

  int ii = 0;
  for (auto& server : this->channels) {
    server.second->write(this->sockfds[ii++], buf, outchars);
  }
}

void df_client::put()
{

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
