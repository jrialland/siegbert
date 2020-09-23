#include "evaluator/Evaluator.hpp"

#include <algorithm>
#include <climits>

#include <iostream>

#define USE_TT 0

using namespace std;

namespace siegbert {

Evaluator::Evaluator()
    : boardstate(BoardState::initial()), transposition_table(3, 102400) {
  scorer = new Berliner1999Scorer();
  moves_sorter = new ShuffleMovesSorter();
}

Evaluator::~Evaluator() {
  delete scorer;
  delete moves_sorter;
}

int Evaluator::minimax(int depth, int alpha, int beta, bool maximizing) {

  // evaluate the score of a leaf node
  if (depth == 0) {
    return scorer->score(boardstate);
  }

  auto memento = boardstate.memento();
  boost::optional<known_score_t> hit = transposition_table.get(memento.z);
  if (hit.is_initialized() && hit->depth <= depth) {
    return hit->score;
  }

  // draw (50 moves rule)
  if (memento.halfmoves == 100) {
    return 0;
  }

  std::vector<Move> moves = boardstate.generate_moves();

  // cannot play further
  if (moves.empty()) {
    int final_score = 0;
    if (boardstate.is_check()) {
      final_score = maximizing ? INT_MIN : INT_MAX;
    }
    transposition_table.set(memento.z, depth, final_score, 0);

    return final_score;
  }

  moves_sorter->sort(moves);

  int score;
  if (maximizing) {
    score = INT_MIN;
    for (auto move : moves) {
      if (boardstate.make_move(move)) {
        score = std::max(score, minimax(depth - 1, alpha, beta, false));
        alpha = std::max(alpha, score);
        boardstate.unmake_move(move, memento);
        if (alpha >= beta) {
          break; // beta cutoff
        }
      }
    }
  } else {
    score = INT_MAX;
    for (auto move : moves) {
      if (boardstate.make_move(move)) {
        score = std::min(score, minimax(depth - 1, alpha, beta, true));
        beta = std::min(beta, score);
        boardstate.unmake_move(move, memento);
        if (beta <= alpha) {
          break; // alpha cutoff
        }
      }
    }
  }
  transposition_table.set(memento.z, depth, score, 0);
  return score;
}

std::string Evaluator::eval(BoardState &b, int depth) {
  boardstate = b;

  std::function<bool(int, int)> compare;
  int best_score;
  Move best_move;

  bool maximizing = boardstate.is_white_to_move();

  // maximizing for white, minimizing for black
  if (maximizing) {
    compare = [](int a, int b) { return b > a; };
  } else {
    compare = [](int a, int b) { return a < b; };
  }

  // store the data for unmake_move()
  auto memento = boardstate.memento();

  std::vector<Move> moves = boardstate.generate_moves();
  moves_sorter->sort(moves);

  // assume that the best move is the first move
  auto it = moves.begin();
  auto end = moves.end();
  while (!boardstate.make_move(*it)) {
    it++;
  }
  best_move = *it;
  best_score = minimax(depth - 1, INT_MIN, INT_MAX, !maximizing);
  boardstate.unmake_move(best_move, memento);

  // for each other moves, do a minimax on it
  while (it != end) {
    if (boardstate.make_move(*it)) {
      int score = minimax(depth - 1, INT_MIN, INT_MAX, !maximizing);
      transposition_table.set(memento.z, depth, score, 0);
      if (compare(best_score, score)) {
        best_score = score;
        best_move = *it;
      }
      boardstate.unmake_move(*it, memento);
    }
    it++;
  }

  /*
  std::cout << "size " << transposition_table.size();
  std::cout << " hits " << transposition_table.hits();
  std::cout << " missed " << transposition_table.missed();
  std::cout << endl;
  */

  return best_move.to_str();
}

} // namespace siegbert