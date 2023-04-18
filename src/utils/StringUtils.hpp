#pragma once
#include <string>
#include <vector>

namespace siegbert {

struct StringUtils {

  static std::vector<std::string> split(const std::string_view &s, char delim);

  static bool iequals(const std::string_view &lhs, const std::string_view &rhs);

  static std::string replace_all(const std::string_view txt,
                                 const std::string_view &replace,
                                 const std::string_view &with);

  static std::string trim(const std::string_view &s);
};

} // namespace siegbert