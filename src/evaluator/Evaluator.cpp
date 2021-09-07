#include "evaluator/Evaluator.hpp"
#include "logging/Logging.hpp"

#include <climits>
#include <vector>
using namespace std;

namespace siegbert {

Evaluator::Evaluator() {}

std::string Evaluator::eval(BoardState &bs, int depth) {
  vector<Move> moves = bs.generate_moves();
  std::string choice = "resign";
  int best = INT_MIN;
  bool has_legal_moves = false;
  auto memento = bs.memento();

  for (auto &move : moves) {
    if (bs.make_move(move)) {
      has_legal_moves = true;
      negamax.set_boardState(bs);

      int score;
      if (bs.white_to_move) {
        score = negamax.negamax(depth, INT_MIN, INT_MAX);
      } else {
        score = negamax.negamax(depth, INT_MIN, INT_MAX);
      }
      bs.unmake_move(move, memento);
      if (score > best) {
        best = score;
        choice = move.to_str();
      }
    }
  }
  return choice;
}

void Evaluator::reset() { negamax.reset(); }
} // namespace siegbert