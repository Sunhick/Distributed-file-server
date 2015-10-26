/****************************************************
 *  df utility function
 *
 * Author : Sunil bn <sunhick@gmail.com>
 *****************************************************/
#ifndef DFS_UTILITIES_H
#define DFS_UTILITIES_H

#include <ostream>
#include <string>
#include <vector>
#include <algorithm>

namespace dfs {

  enum color {
    FG_RED      = 31,
    FG_GREEN    = 32,
    FG_BLUE     = 34,
    FG_DEFAULT  = 39,
    BG_RED      = 41,
    BG_GREEN    = 42,
    BG_BLUE     = 44,
    BG_DEFAULT  = 49
  };

  class modifier {
    color code;
  public:
  modifier(color code) : code(code) {}
    friend std::ostream& operator<<(std::ostream& os, const modifier& mod) {
      return os << "\033[" << mod.code << "m";
    }
  };

  class utilities {
  public:
    static std::vector<std::string> split(const std::string& str, int delimiter(int),
					  int occurances = -1); // -1: look for all occurances
  };
}

#endif
