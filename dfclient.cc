/****************************************************
 *  df client implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "dfclient.h"

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <string.h>

#include "include/dfcomm.h"

using namespace dfs;

int main(int argc, char *argv[])
{
  std::cout << "DF Client"  << "\n";
  std::string host = "localhost";  
  char	*portnum = argv[1];
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
  return 0;
}
