/****************************************************
 *  df utility function
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfutilities.h"

using namespace dfs;

std::vector<std::string> utilities::split(const std::string& str, int delimiter(int))
{
  std::vector<std::string> result;
  auto end = str.end();
  auto start = str.begin();
  while (start != end) {
    start = find_if_not(start, end, delimiter);
    if (start == end) break;
    auto index = find_if(start, end, delimiter);
    result.push_back(std::string(start,index));
    start = index;
  }

  return result;
}
