/****************************************************
 *  df utility function
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_UTILITIES_H
#define DFS_UTILITIES_H

#include <string>
#include <vector>
#include <algorithm>

namespace dfs {

  class utilities {
  public:
    static std::vector<std::string> split(const std::string& str, int delimiter(int));
  };
}

#endif
