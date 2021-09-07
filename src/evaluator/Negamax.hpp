#include "evaluator/Scorer.hpp"
#include "evaluator/TranspositionTable.hpp"
#include "game/BoardState.hpp"

namespace siegbert {

class Negamax {
private:
  TranspositionTable tt;

  Scorer scorer;

  BoardState *pBs;

public:
  void set_boardState(BoardState &boardState);

  int negamax(int depth, int alpha, int beta);

  void reset();
};

} // namespace siegbert