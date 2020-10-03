#include "evaluator/Evaluator.hpp"

#include <algorithm>
#include <climits>

#include <iostream>

#define USE_TT 0

using namespace std;

namespace siegbert {

Evaluator::Evaluator() :
  thread_pool(),
  transposition_table(3, 102400) {
  scorer = new Berliner1999Scorer();
  moves_sorter = new ShuffleMovesSorter();
}

Evaluator::~Evaluator() {
  delete scorer;
  delete moves_sorter;
}

int Evaluator::minimax(
  BoardState& boardstate,
  int depth,
  int alpha,
  int beta,
  bool maximizing,
  uint64_t z_pos4,
  uint64_t z_pos3,
  uint64_t z_pos2,
  uint64_t z_pos1
) {

  // evaluate the score if a leaf node
  if (depth == 0) {
    return scorer->score(boardstate);
  }

  auto memento = boardstate.memento();

  // 3 fold repetition
  if(z_pos2 == z_pos4 && memento.z == z_pos2) {
    return 0;
  }

  // draw (50 non-pawn, non-capture moves)
  if (memento.halfmoves == 100) {
    return 0;
  }

  // if the position is already in the transposition table, return its score
  boost::optional<known_score_t> hit = transposition_table.get(memento.z);
  if (hit.is_initialized() && hit->depth <= depth) {
    return hit->score;
  }

  std::vector<Move> moves = boardstate.generate_moves();

  // cannot play further
  if (moves.empty()) {
    int final_score = 0;

    // we are checkmated
    if (boardstate.is_check()) {
      final_score = maximizing ? INT_MIN : INT_MAX;
    } // else this is a pat
    transposition_table.set(memento.z, depth, final_score, 0);
    return final_score;
  }

  moves_sorter->sort(moves);

  int score;
  if (maximizing) {
    score = INT_MIN;
    for (auto move : moves) {
      if (boardstate.make_move(move)) {
        score = std::max(
          score, 
          minimax(
            boardstate,
            depth - 1,
            alpha,
            beta,
            false,
            z_pos3,
            z_pos2,
            z_pos1,
            memento.z
          )
        );
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
        score = std::max(
          score, 
          minimax(
            boardstate,
            depth - 1,
            alpha,
            beta,
            true,
            z_pos3,
            z_pos2,
            z_pos1,
            memento.z
          )
        );
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

struct Eval {
  Move move;
  int score;
};

std::string Evaluator::eval(BoardState &b, int depth) {

  vector<Future<Eval>*> evals;
  vector<Move> moves = b.generate_moves();

  if(moves.empty()) {
    if(b.is_check()) {
      return b.is_white_to_move() ? "0/1":"1/0";
    } else {
      return "1/2-1/2";
    }
  }

  moves_sorter->sort(moves);

  Move bestMove;
  int bestScore = b.is_white_to_move() ? INT_MIN : INT_MAX;

  Memento memento = b.memento();
  uint64_t z = memento.z;

  for(const Move& move : moves) {
    if(b.make_move(move)) {
      bestMove = move; // for the moment

      std::function<Eval(void)> evalFn = [this, b, z, depth, &move](void) -> Eval {
        Eval eval;
        eval.move = move;
        BoardState root(b);
        eval.score = minimax(
            root,
            depth,
            INT_MIN,
            INT_MAX,
            root.is_white_to_move(),
            4,
            3,
            2,
            z
        );
        return eval;
      };
      evals.push_back(thread_pool.submit(evalFn));
      b.unmake_move(move, memento);
    }
  }

  std::function<bool(int, int)> compare;
  if(b.is_white_to_move()) {
    compare = [](int a, int b) -> bool { return a > b; };
  } else {
    compare = [](int a, int b) -> bool { return a < b; };
  }

  for(auto f : evals) {
    Eval e = f->get();
    if(compare(e.score, bestScore)) {
      bestScore = e.score;
      bestMove = e.move;
    }
    delete f;
  }
  return bestMove.to_str();
}

}