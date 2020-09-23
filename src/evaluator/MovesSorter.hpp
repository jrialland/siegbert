#ifndef MovesSorter_HPP
#define MovesSorter_HPP

#include "game/BoardState.hpp"
#include <vector>

namespace siegbert {

class MovesSorter {

public:
  virtual void sort(std::vector<Move> &moves) const = 0;
};

class NullMovesSorter : public MovesSorter {

public:
  void sort(std::vector<Move> &moves) const override;
};

class ShuffleMovesSorter : public MovesSorter {

public:
  void sort(std::vector<Move> &moves) const override;
};

} // namespace siegbert

#endif