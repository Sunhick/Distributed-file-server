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

using namespace dfs;

df_client::df_client(std::string& file)
{
  this->config = new dfconfig(file);
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
  for (auto server : this->channels) {
    auto id = server.second->connect();
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
  
  /*
  std::string host = "localhost";  
  char	*portnum = argv[1];

  dfconfig config("dfc.conf");
  generic_comm* comm = new df_socket_comm(host, atoi(portnum));
  int sockfd = comm->connect(); 
  
  while (true) {
    std::cout << "1. LIST 2. PUT 3. GET 4. EXIT " << std::endl;
    int choice;
    std::cin >> choice;

    switch(choice) {
    case 4:
      {
	close(sockfd);
	exit(0);
      }
      
    case 3:
      {
	system("clear");
	std::cout << "Enter file name to get:";
	std::string file("");
	std::cin >> file;
	char buf[128] = "GET hello.txt\0";
	auto outchars = strlen(buf);
	write(sockfd, buf, outchars);
      }
      break;
      
    case 2:
      {
      }
      break;
      
    case 1:
      {
      }
      break;
    }
  }

  delete comm;
  */
  return 0;
}
