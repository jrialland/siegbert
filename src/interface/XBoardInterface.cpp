
#include "interface/XBoardInterface.hpp"

#include <boost/regex.hpp>

namespace siegbert {

XBoardInterface::XBoardInterface(EngineIO *io) {
  engineIO = io;
  handlers["xboard"] = [this] {
    engineIO->send("tellics say    siegbert 0.1");
    engineIO->send("tellics say    (c) Julien Rialland, All rights reserved.");
  };

  handlers["protover 2"] = [this] {
    engineIO->send("feature myname=\"siegbert\"");
    engineIO->send("feature ping=1");
    engineIO->send("feature san=0");
    engineIO->send("feature sigint=0");
    engineIO->send("feature sigterm=0");
    engineIO->send("feature setboard=1");
    engineIO->send("feature debug=1");
    engineIO->send("feature time=0");
    engineIO->send("feature playother=0");
    engineIO->send("feature done=1");
  };

  handlers["force"] = [this] { force = true; };

  handlers["new"] = [this] { boardstate = BoardState::initial(); };

  handlers["go"] = [this] {
    force = false;
    play();
  };

  handlers["undo"] = [this] { handlers["remove"](); };

  handlers["remove"] = [this] {
    if (history.size() > 0) {
      auto prev = history.back();
      boardstate.unmake_move(prev.first, prev.second);
      history.pop_back();
    }
  };

  handlers["fen"] = [this] { engineIO->send(boardstate.to_fen()); };

  handlers["quit"] = [this] { exit_required_ = true; };

  handlers["white"] = [this] { boardstate.set_white_to_move(true); };

  handlers["black"] = [this] { boardstate.set_white_to_move(false); };
}

static const boost::regex re_move("^[a-h][1-8][a-h][1-8][kbrq]?$");
static const boost::regex re_ping("^ping ([0-9a-z]+)$");
static const boost::regex re_setboard("^setboard (.+)$");

void XBoardInterface::receive(const std::string &line) {
  auto it = handlers.find(line);

  if (it != handlers.end()) {
    it->second();
    return;
  }

  boost::cmatch m;

  if (regex_match(line.c_str(), m, re_move)) {
    boardstate_memento_t memento = boardstate.memento();
    Move m = boardstate.get_move(line);
    history.push_back(std::make_pair(m, memento));
    boardstate.make_move(m);
    if (!force) {
      play();
    }
    return;
  }

  else if (regex_match(line.c_str(), m, re_ping)) {
    std::string val;
    val.assign(m[1].first, m[1].second);
    engineIO->send(std::string("pong ") + val);
  }

  else if (regex_match(line.c_str(), m, re_setboard)) {
    std::string fen;
    fen.assign(m[1].first, m[1].second);
    boardstate = BoardState::from_fen(fen);
    history.clear();
  }
}

void XBoardInterface::play() {
  std::string move = evaluator.eval(boardstate, 7);
  engineIO->send("move " + move);
  boardstate.make_move(boardstate.get_move(move));
}

} // namespace siegbert