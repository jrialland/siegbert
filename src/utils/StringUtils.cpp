#include "StringUtils.hpp"
#include <algorithm>
#include <sstream>

namespace siegbert {

std::vector<std::string> StringUtils::split(const std::string_view &s,
                                            char delim) {
  std::vector<std::string> elems;
  std::string splitted{s.begin(), s.end()};
  std::stringstream ss(splitted);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}

bool StringUtils::iequals(const std::string_view &lhs,
                          const std::string_view &rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                    [](char a, char b) { return tolower(a) == tolower(b); });
}

std::string StringUtils::replace_all(const std::string_view txt,
                                     const std::string_view &replace,
                                     const std::string_view &with) {
  std::string result{txt.begin(), txt.end()};
  std::string::size_type pos = 0;
  while ((pos = result.find(replace, pos)) != std::string::npos) {
    result.replace(pos, replace.length(), with);
    pos += with.length();
  }
  return result;
}

std::string StringUtils::trim(const std::string_view &s) {
  auto wsfront = std::find_if_not(s.begin(), s.end(),
                                  [](int c) { return std::isspace(c); });
  auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) {
                  return std::isspace(c);
                }).base();
  return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

} // namespace siegbert