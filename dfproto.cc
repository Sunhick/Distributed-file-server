/****************************************************
 *  Distributed file communication protocol
 *  implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfproto.h"
#include "include/dfutils.h"

#include <stdlib.h>

#include <vector>
#include <sstream>

using namespace dfs;

df_request_proto::df_request_proto(const std::string& name,
				   const std::string& password)
{
  this->username = name;
  this->password = password;
}

df_request_proto::~df_request_proto()
{
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

df_reply_proto::df_reply_proto(std::string reply) 
{
  auto parsed = utilities::split(reply, [](int c) { return (c=='|' ? 1 : 0); });

  this->ecode = atoi(parsed[0].c_str());
  this->emsg = parsed[1];
  this->contents = parsed[2];
}

df_reply_proto::df_reply_proto(int ecode,
			       std::string emsg,
			       std::string content) 
{
  this->ecode = ecode;
  this->emsg = emsg;
  this->contents = content;
}

df_reply_proto::~df_reply_proto() 
{

}

std::string df_reply_proto::to_string()
{
  std::stringstream data;
  data << ecode << "|" << emsg << "|" << contents;
  return data.str();
}
