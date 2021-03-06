/****************************************************
 *  df configuration parser implementation file
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfconfig.h"

#include <iostream>
#include <fstream>
#include <cstdlib>

#include "include/dfutils.h"

using namespace dfs;

dfconfig::dfconfig(std::string fpath) 
{
  std::ifstream cfile(fpath.c_str());
  if (!cfile.good())
    throw "File doesn't exists!";

  for (std::string str; getline(cfile, str);) {
    if (str[0] == '#') // comment line, skip it.
      continue;

    // split based on space delimiter
    auto tokens = utilities::split(str, std::isspace);
    auto identifier = tokens[0];
    std::transform(identifier.begin(), identifier.end(), identifier.begin(), ::tolower);
    if (identifier != "server" &&
	(identifier == "username:" || identifier == "password")) {
      // collect user name and password
      getline(cfile, str);
      auto password = utilities::split(str, std::isspace).at(1);
      auto credentials = std::pair<std::string, std::string>(tokens[1], password);
      this->authentications.insert(credentials);
      continue;
    }

    std::string dfs = tokens.at(2);
    // split based on colon delimiter to get ip address and port
    auto address = utilities::split(dfs, [](int ch){ return (ch == ':'? 1 : 0); });
    struct dfs_address addr = {address[0], atoi(address[1].c_str())};

    this->chunk_servers.insert(std::pair<std::string, dfs_address>(tokens[1], addr));
  }

#ifdef DEBUG
  std::cout << chunk_servers.size() << std::endl;
  for (auto const &srv : chunk_servers) {
    std::cout << srv.first << " => " << srv.second.ip_address << ":" << srv.second.port << std::endl;
  }

  std::cout << authentications.size() << std::endl;
  for (auto const &auth : authentications) {
    std::cout << auth.first << " => " << auth.second << std::endl;
  }
#endif
}

dfconfig::~dfconfig() 
{
  this->chunk_servers.clear();
  this->authentications.clear();
}

bool dfconfig::validate(std::string name, std::string password) 
{
  return (this->authentications.find(name) != this->authentications.end()
    && password == this->authentications[name]);
}

dfs_address dfconfig::get_chunk_server(std::string name) 
{
  return 
    (this->chunk_servers.find(name) != this->chunk_servers.end()) ?
    this->chunk_servers[name] : dfs_address();
    // throw "specified distributed server is not running!";
}

std::map<std::string, dfs_address> dfconfig::get_all_servers()
{
  return this->chunk_servers;
}

void dfconfig::get_username_password(std::string& name,
				     std::string& password,
				     unsigned int index)
{
  // index out of bounds
  if (index < 0 || index > this->authentications.size())
    return;
  for (auto &pair : authentications) {
    name = pair.first;
    password = pair.second;
    break;
  }
}
