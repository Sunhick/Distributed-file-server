/****************************************************
 *  df utility function
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#include "include/dfutils.h"

using namespace dfs;

std::vector<std::string> utilities::split(const std::string& str, int delimiter(int), int occurances)
{
  std::vector<std::string> result;
  auto end = str.end();
  auto start = str.begin();
  while (start != end && occurances) {
    occurances--;
    start = find_if_not(start, end, delimiter);
    if (start == end) break;
    auto index = find_if(start, end, delimiter);
    if (occurances == 0)
      result.push_back(std::string(start--, end));
    else
      result.push_back(std::string(start, index));
    start = index;
  }

  return result;
}
