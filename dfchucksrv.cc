/****************************************************
 *  Distributed file chunk server implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfchunksrv.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define PERM_MASK 0777

using namespace dfs;

df_chunk_srv::df_chunk_srv(std::string filesys, int port) 
{
  if (filesys.empty())
    throw std::string("File system is not mentioned");  

  this->port = port;
  this->filesys = filesys;
  struct stat st = {0};
  // create a File system if it doesn't exists
  if (stat(filesys.c_str(), &st) == -1) {
    mkdir(filesys.c_str(), PERM_MASK);
  }
}

df_chunk_srv::~df_chunk_srv() 
{
  
}

void df_chunk_srv::list()
{
  
}

void df_chunk_srv::get(std::string filename)
{
  
}

void df_chunk_srv::put(std::string filename, std::string content)
{
}
