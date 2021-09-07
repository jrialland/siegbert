#ifndef UciInterface_HPP
#define UciInterface_HPP

#include "game/BoardState.hpp"
#include "interface/EngineIO.hpp"
#include "interface/EngineInterface.hpp"

#include <functional>
#include <map>

namespace siegbert {

class UciInterface : public EngineInterface {

private:

  EngineIO *io;

  BoardState boardState = BoardState::initial();

  std::map<std::string, std::function<void()>> handlers;

  void set_option(const std::string &key, const std::string &value);

public:

  UciInterface(EngineIO *io);

  void receive(const std::string &line) override;

  void set_position(const std::string &pos,
                    const std::vector<std::string> &moves);

  void go();
};
} // namespace siegbert

#endif