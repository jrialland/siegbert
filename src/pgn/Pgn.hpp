#ifndef PGN_HPP
#define PGN_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

namespace siegbert {

class Pgn {

private:
  static const int BLANK = 1;
  static const int HEADERS = 2;
  static const int GAME = 3;

  static const boost::regex re_space;
  static const boost::regex re_number;

  void add_tag(const std::string &tag) {
    int firstspace = tag.find_first_of(' ');
    std::string key = tag.substr(1, firstspace - 1);
    std::string value = tag.substr(firstspace + 2);
    value = value.substr(0, value.find_last_of('"'));
    tags[key] = value;
  }

  void set_text(std::string &pgntxt) {
    boost::sregex_token_iterator i(pgntxt.begin(), pgntxt.end(), re_space, -1);
    boost::sregex_token_iterator end;
    std::vector<std::string> tokens;
    while (i != end) {
      std::string token = *i++;
      if (token.length() > 0) {
        tokens.push_back(token);
      }
    }

    if (tokens.back().compare("1-0") == 0 ||
        tokens.back().compare("0-1") == 0 ||
        tokens.back().compare("1/2-1/2") == 0 ||
        tokens.back().compare("*") == 0) {
      tokens.pop_back();
    }

    boost::cmatch m;
    for (std::string t : tokens) {
      if (regex_match(t.c_str(), m, re_number) || t.compare("...") == 0) {
        continue;
      } else {
        moves.push_back(t);
      }
    }
  }

public:
  std::map<std::string, std::string> tags;

  std::vector<string> moves;

  static std::vector<Pgn> read_file(const std::string &filename) {
    std::ifstream in(filename);
    if (!in.good()) {
      throw std::invalid_argument("filename");
    }
    return read(in, filename);
  }

  static std::vector<Pgn> read(std::istream &in, const std::string &filename) {
    std::vector<Pgn> pgns;
    std::string line;
    std::string pgntxt;
    int state = BLANK;
    int lineno = 0;
    while (std::getline(in, line)) {

      lineno++;
      boost::trim(line);

      // detect start of game
      if (state != HEADERS && line[0] == '[') {
        state = HEADERS;
        pgns.push_back(Pgn());
        pgns.back().tags["_filename"] = filename;
        pgns.back().tags["_line"] = std::to_string(lineno);
        pgntxt.clear();
      }

      // tag line
      if (state == HEADERS && line[0] == '[') {
        pgns.back().add_tag(line);
        continue;
      }

      // detect end of tags
      if (state == HEADERS && line.length() == 0) {
        state = GAME;
        continue;
      }

      // game text
      if (state == BLANK && line.length() > 0 && (line[0] >= '1') &&
          (line[0] <= '9')) {
        pgns.push_back(Pgn());
        state = GAME;
        pgntxt += " " + line;
        continue;
      }

      if (state == GAME && line.length() > 0) {
        pgntxt += " " + line;
        continue;
      }

      // end of game
      if (state == GAME && line.length() == 0) {
        pgns.back().set_text(pgntxt);
        state = BLANK;
        continue;
      }
    }
    if (state != BLANK) {
      pgns.back().set_text(pgntxt);
    }
    return pgns;
  }
};

std::ostream &operator<<(std::ostream &os, const Pgn &pgn);

} // namespace siegbert

#endif