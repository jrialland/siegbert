
#include <regex>
using namespace std;

#include "interface/UciInterface.hpp"
#include "utils/StringUtils.hpp"

namespace siegbert {

UciInterface::UciInterface(EngineIO *io_) : io(io_) {

  handlers["uci"] = [this] {
    io->send("id name siegbert");
    io->send("id author Julien Rialland <julien.rialland@gmail.com>");
    io->send("option name OwnBook type check default true");
    io->send("uciok");
  };

  handlers["isready"] = [this] { io->send("readyok"); };

  handlers["quit"] = [this] { exit_required_ = true; };

  handlers["ucinewgame"] = [this] { boardState = BoardState::initial(); };

  handlers["ponderhit"] = [this] {
    // TODO
  };
}

static const std::regex re_setoption("^setoption name (.+) value (.+)");
static const std::regex re_register("^register (.+)$");
static const std::regex re_position("^position (.+)$");
static const std::regex re_go("^go( .*)$");

void UciInterface::receive(const string &line) {

  auto it = handlers.find(line);
  if (it != handlers.end()) {
    it->second();
    return;
  }

  std::cmatch m;

  // setoption xxx yyyy
  if (std::regex_match(line.c_str(), m, re_setoption)) {
    string name;
    string value;
    name.assign(m[1].first, m[1].second);
    value.assign(m[2].first, m[2].second);
    set_option(name, value);
    return;
  }

  // position startpos|fen moves ...
  if (std::regex_match(line.c_str(), m, re_position)) {
    string params;
    params.assign(m[1].first, m[1].second);
    vector<string> parts = StringUtils::split(params, ' ');
    vector<string> moves;
    string pos;
    int ni;

    if (parts[0].compare("startpos") == 0) {
      pos = parts[0];
      ni = 1;
    } else if (parts[0].compare("fen") == 0) {
      // FEN string
      pos = parts[1] + " " + parts[2] + " " + parts[3] + " " + parts[4] + " " +
            parts[5] + " " + parts[6];
      ni = 6;
    }

    if (parts.size() > 1 + ni && parts[ni].compare("moves") == 0) {
      for (int i = 1 + ni; i < parts.size(); i++) {
        moves.push_back(parts[i]);
      }
    }
    set_position(pos, moves);
    return;
  }

  if (std::regex_match(line.c_str(), m, re_go)) {
    string params;
    params.assign(m[1].first, m[1].second);
    vector<string> parts = StringUtils::split(params, ' ');
    for (int i = 0; i < parts.size(); i += 1) {
      // TODO
    }
    go();
    return;
  }

  io->send("info unrecognized command");
}

void UciInterface::set_position(const string &pos,
                                const vector<string> &moves) {

  if (pos.compare("startpos") == 0) {
    boardState = BoardState::initial();
  } else {
    boardState = BoardState::from_fen(pos);
  }

  for (auto move : moves) {
    boardState.make_move(boardState.get_move(move));
  }
}

void UciInterface::set_option(const string &key, const string &value) {}

void UciInterface::go() {}
} // namespace siegbert