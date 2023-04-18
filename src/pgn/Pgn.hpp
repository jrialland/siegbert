#ifndef PGN_HPP
#define PGN_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>
using namespace std;

namespace siegbert {

class Pgn {

private:
  static const int BLANK = 1;
  static const int HEADERS = 2;
  static const int GAME = 3;

  static const regex re_space;
  static const regex re_number;

  void add_tag(const std::string &tag);

  void set_text(std::string &pgntxt);

public:
  std::map<std::string, std::string> tags;

  std::vector<string> moves;

  static std::vector<Pgn> read(std::istream &in, const std::string &filename);

  static std::vector<Pgn> read_file(const std::string &filename);
};

std::ostream &operator<<(std::ostream &os, const Pgn &pgn);

} // namespace siegbert

#endif