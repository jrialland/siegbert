#ifndef EngineInterface_HPP
#define EngineInterface_HPP

#include <string>

#include "evaluator/Evaluator.hpp"
#include "game/BoardState.hpp"

namespace siegbert {

class EngineIO;

class EngineInterface {

protected:

  BoardState boardstate = BoardState::initial();

  Evaluator evaluator;

  EngineIO *engineIO;

  bool exit_required_ = false;

public:
  virtual void receive(const std::string &line) = 0;

  bool is_exit_required() const { return exit_required_; }
};

} // namespace siegbert

#endif