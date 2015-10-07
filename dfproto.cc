/****************************************************
 *  Distributed file communication protocol
 *  implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfproto.h"
#include <sstream>

using namespace dfs;

df_request_proto::df_request_proto(const std::string& name,
				   const std::string& password)
{
  this->username = name;
  this->password = password;
}

void df_request_proto::set_command(const std::string& cmd,
				   const std::string& args)
{
  this->command = cmd;
  this->arguments = args;
}

std::string df_request_proto::to_string()
{
  std::stringstream data;
  if (!arguments.empty())
    data << username << "|" << password << "|" << command << "|" << arguments;
  else
    data << username << "|" << password << "|" << command;
  return data.str();
}
