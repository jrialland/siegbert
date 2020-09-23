
#include "pgn/Pgn.hpp"

namespace siegbert {

const boost::regex Pgn::re_space("\\.|\\s+");

const boost::regex Pgn::re_number("^[0-9]+$");

std::ostream &operator<<(std::ostream &os, const Pgn &pgn) {
  std::string filename = pgn.tags.find("_filename")->second;
  std::string line = pgn.tags.find("_line")->second;
  os << "pgn<\"" << filename << "\":" << line << ">";
  return os;
}

} // namespace siegbert