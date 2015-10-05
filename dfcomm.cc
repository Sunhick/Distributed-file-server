/****************************************************
 *  df communication implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfcomm.h"

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

using namespace dfs;

df_socket_comm::df_socket_comm(std::string host, int port) : generic_comm()
{
  this->port = port;
  this->host = host;
}

df_socket_comm::~df_socket_comm()
{
}

int df_socket_comm::die(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  exit(EXIT_FAILURE);  
}

int df_socket_comm::open(int backlog)
{
  struct sockaddr_in sin; 	// an Internet endpoint address 
  int sockfd;               	// socket descriptor 

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;

#ifdef USE_IP
  // support for connecting from different host
  sin.sin_addr.s_addr = inet_addr("10.201.29.217");
#else
  // use local host
  sin.sin_addr.s_addr = INADDR_ANY;
#endif

  // Map port number (char string) to port number (int)
  if ((sin.sin_port=htons((unsigned short)this->port)) == 0)
    die("can't get \"%sockfd\" port number\n", this->port);

  // Allocate a socket
  sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockfd < 0)
    die("can't create socket: %sockfd\n", strerror(errno));

  // Bind the socket
  if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    fprintf(stderr, "can't bind to %d port: %s; Trying other port\n",
	    this->port, strerror(errno));
    sin.sin_port=htons(0); /* request a port number to be allocated
			      by bind */
    if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      die("can't bind: %sockfd\n", strerror(errno));
    } else {
      int socklen = sizeof(sin);
      if (getsockname(sockfd, (struct sockaddr *)&sin, (socklen_t *)&socklen) < 0)
	die("getsockname: %sockfd\n", strerror(errno));
      printf("New server port number is %d\n", ntohs(sin.sin_port));
    }
  }

  if (listen(sockfd, backlog) < 0)
    die("can't listen on %sockfd port: %sockfd\n", this->port, strerror(errno));

  return sockfd;
}

int df_socket_comm::connect()
{
  struct hostent  *host_entry;   // pointer to host information entry
  struct sockaddr_in sin; // an Internet endpoint address
  int     sockd;              // socket descriptor


  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;

  auto portnum = this->port;
  auto host = this->host.c_str();

  // Map port number (char string) to port number (int)
  if ((sin.sin_port=htons((unsigned short)portnum)) == 0)
    die("can't get \"%sockd\" port number\n", portnum);

  // Map host name to IP address, allowing for dotted decimal
  if ((host_entry = gethostbyname(host)))
    memcpy(&sin.sin_addr, host_entry->h_addr, host_entry->h_length);
  else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
    die("can't get \"%sockd\" host entry\n", host);

  // Allocate a socket
  sockd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sockd < 0)
    die("can't create socket: %sockd\n", strerror(errno));

  // Connect the socket
  if (::connect(sockd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    die("can't connect to %sockd.%sockd: %sockd\n", host, portnum,
	strerror(errno));
  return sockd;
}

void df_socket_comm::read()
{

}

void df_socket_comm::write()
{

}
