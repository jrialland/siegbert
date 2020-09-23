#include "evaluator/MovesSorter.hpp"

#include <algorithm>

namespace siegbert {

void NullMovesSorter::sort(std::vector<Move> &moves) const {}

void ShuffleMovesSorter::sort(std::vector<Move> &moves) const {
  std::random_shuffle(moves.begin(), moves.end());
}

} // namespace siegbert
