#ifndef Evaluator_HPP
#define Evaluator_HPP

#include "evaluator/MovesSorter.hpp"
#include "evaluator/Scorer.hpp"
#include "evaluator/TranspositionTable.hpp"
#include "game/BoardState.hpp"

namespace siegbert {
class Evaluator {

protected:
  BoardState boardstate;

  TranspositionTable transposition_table;

  Scorer *scorer;

  MovesSorter *moves_sorter;

  int minimax(int depth, int alpha, int beta, bool maximizing);

public:
  Evaluator();

  ~Evaluator();

  void start_eval(const BoardState &boardState);

  std::string eval(BoardState &boardState, int depth);

  void stop();
};
} // namespace siegbert
#endif