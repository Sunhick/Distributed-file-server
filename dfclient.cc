/****************************************************
 *  df client implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "dfclient.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <string.h>

#include "include/dfcomm.h"

using namespace dfs;

int errexit(const char *format, ...)
{
  va_list args;

  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  exit(1);
}

int connect_sock(const char *host, const char *portnum)
{
  struct hostent  *host_entry;   // pointer to host information entry
  struct sockaddr_in sin; // an Internet endpoint address
  int     sockd;              // socket descriptor


  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;

  // Map port number (char string) to port number (int)
  if ((sin.sin_port=htons((unsigned short)atoi(portnum))) == 0)
    errexit("can't get \"%sockd\" port number\n", portnum);

  // Map host name to IP address, allowing for dotted decimal
  if ((host_entry = gethostbyname(host)))
    memcpy(&sin.sin_addr, host_entry->h_addr, host_entry->h_length);
  else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
    errexit("can't get \"%sockd\" host entry\n", host);

  // Allocate a socket
  sockd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockd < 0)
    errexit("can't create socket: %sockd\n", strerror(errno));

  // Connect the socket
  if (connect(sockd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    errexit("can't connect to %sockd.%sockd: %sockd\n", host, portnum,
	    strerror(errno));
  return sockd;
}


int main(int argc, char *argv[])
{
  std::cout << "DF Client"  << "\n";
  std::string host = "localhost";  
  char	*portnum = argv[1];
  generic_comm* comm = new df_socket_comm(host, atoi(portnum));
  int sockfd = comm->connect(); // connect_sock(host.c_str(), portnum);
  
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
