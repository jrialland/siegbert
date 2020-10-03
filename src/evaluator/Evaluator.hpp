#ifndef Evaluator_HPP
#define Evaluator_HPP

#include "evaluator/MovesSorter.hpp"
#include "evaluator/Scorer.hpp"
#include "evaluator/TranspositionTable.hpp"
#include "game/BoardState.hpp"
#include "threading/threading.hpp"

namespace siegbert {
class Evaluator {

protected:

  ThreadPool thread_pool;

  TranspositionTable transposition_table;

  Scorer *scorer;

  MovesSorter *moves_sorter;

  int minimax(
    BoardState& boardstate,
    int depth,
    int alpha,
    int beta,
    bool maximizing,
    uint64_t z_pos4,
    uint64_t z_pos3,
    uint64_t z_pos2,
    uint64_t z_pos1
    );

public:

  Evaluator();

  ~Evaluator();

  std::string eval(BoardState &boardState, int depth);

  void stop();
};
} // namespace siegbert
#endif