
#include "pgn/Pgn.hpp"
#include "utils/StringUtils.hpp"

namespace siegbert {

const regex Pgn::re_space("\\.|\\s+");

const regex Pgn::re_number("^[0-9]+$");

void Pgn::add_tag(const std::string &tag) {
  int firstspace = tag.find_first_of(' ');
  std::string key = tag.substr(1, firstspace - 1);
  std::string value = tag.substr(firstspace + 2);
  value = value.substr(0, value.find_last_of('"'));
  tags[key] = value;
}

void Pgn::set_text(std::string &pgntxt) {
  sregex_token_iterator i(pgntxt.begin(), pgntxt.end(), re_space, -1);
  sregex_token_iterator end;
  std::vector<std::string> tokens;
  while (i != end) {
    std::string token = *i++;
    if (token.length() > 0) {
      tokens.push_back(token);
    }
  }

  if (tokens.back().compare("1-0") == 0 || tokens.back().compare("0-1") == 0 ||
      tokens.back().compare("1/2-1/2") == 0 ||
      tokens.back().compare("*") == 0) {
    tokens.pop_back();
  }

  std::cmatch m;
  for (std::string t : tokens) {
    if (std::regex_match(t.c_str(), m, re_number) || t.compare("...") == 0) {
      continue;
    } else {
      moves.push_back(t);
    }
  }
}

std::vector<Pgn> Pgn::read(std::istream &in, const std::string &filename) {
  std::vector<Pgn> pgns;
  std::string line;
  std::string pgntxt;
  int state = BLANK;
  int lineno = 0;
  while (std::getline(in, line)) {

    lineno++;
    line = StringUtils::trim(line);

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

std::vector<Pgn> Pgn::read_file(const std::string &filename) {
  std::ifstream in(filename);
  if (!in.good()) {
    throw std::invalid_argument("filename");
  }
  return read(in, filename);
}

std::ostream &operator<<(std::ostream &os, const Pgn &pgn) {
  std::string filename = pgn.tags.find("_filename")->second;
  std::string line = pgn.tags.find("_line")->second;
  os << "pgn<\"" << filename << "\":" << line << ">";
  return os;
}

} // namespace siegbert