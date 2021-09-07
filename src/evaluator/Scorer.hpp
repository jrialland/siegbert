#pragma once
#include "game/BoardState.hpp"

namespace siegbert {

class Scorer {

public:
  /** !! signed score ( should return a value <0 if better for black) */
  int getScore(BoardState &boardState);
};

} // namespace siegbert