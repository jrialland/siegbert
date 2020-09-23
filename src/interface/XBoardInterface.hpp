#ifndef XBoardInterface_HPP
#define XBoardInterface_HPP

#include <functional>
#include <map>
#include <vector>

#include "evaluator/Evaluator.hpp"
#include "interface/EngineIO.hpp"
#include "interface/EngineInterface.hpp"

namespace siegbert {

class XBoardInterface : public EngineInterface {

private:
  bool force = false;

  std::vector<std::pair<Move, Memento>> history;

  std::map<std::string, std::function<void()>> handlers;

  void play();

public:
  XBoardInterface(EngineIO *io);

  void receive(const std::string &line) override;
};
} // namespace siegbert

#endif