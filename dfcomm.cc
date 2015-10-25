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
    die("can't get \"%socketfd\" port number\n", this->port);

  // Allocate a socket
  socketfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socketfd < 0)
    die("can't create socket: %socketfd\n", strerror(errno));

  // Bind the socket
  if (bind(socketfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    fprintf(stderr, "can't bind to %d port: %s; Trying other port\n",
	    this->port, strerror(errno));
    sin.sin_port=htons(0); /* request a port number to be allocated
			      by bind */
    if (bind(socketfd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
      die("can't bind: %socketfd\n", strerror(errno));
    } else {
      int socklen = sizeof(sin);
      if (getsockname(socketfd, (struct sockaddr *)&sin, (socklen_t *)&socklen) < 0)
	die("getsockname: %socketfd\n", strerror(errno));
      printf("New server port number is %d\n", ntohs(sin.sin_port));
    }
  }

  if (listen(socketfd, backlog) < 0)
    die("can't listen on %socketfd port: %socketfd\n", this->port, strerror(errno));

  return socketfd;
}

int df_socket_comm::connect()
{
  struct hostent  *host_entry;   // pointer to host information entry
  struct sockaddr_in sin;        // an Internet endpoint address

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
  socketfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socketfd < 0)
    die("can't create socket: %sockd\n", strerror(errno));

  // Connect the socket
  if (::connect(socketfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    die("can't connect to %sockd.%sockd: %sockd\n", host, portnum, strerror(errno));
  return socketfd;
}

ssize_t df_socket_comm::read(int fd, void* buf, size_t count)
{
  return ::read(fd, buf, count);
}

ssize_t df_socket_comm::read(void* buf, size_t count)
{
  return read(socketfd, buf, count);
}

ssize_t df_socket_comm::write(int fd, const void *buf, size_t count)
{
  return ::write(fd, buf, count);
}

ssize_t df_socket_comm::write(const void *buf, size_t count) 
{
  return write(socketfd, buf, count);
}

int df_socket_comm::accept(int sockfd)
{
  struct sockaddr_in their_addr;
  socklen_t sin_size = sizeof(struct sockaddr_in);
  return ::accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
}
