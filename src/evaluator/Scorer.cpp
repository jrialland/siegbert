#include "evaluator/Scorer.hpp"

#include <iostream>
using namespace std;

namespace siegbert {

#define PAWN_VAL 100
#define KNIGHT_VAL 320
#define BISHOP_VAL 333
#define ROOK_VAL 510
#define QUEEN_VAL 880

int Berliner1999Scorer::score(const BoardState &boardstate) {
  PiecesCount count = boardstate.count_pieces();

  centipawns_t white_score =
      PAWN_VAL * count.white_pawns + KNIGHT_VAL * count.white_knights +
      BISHOP_VAL * count.white_bishops + ROOK_VAL * count.white_rooks +
      QUEEN_VAL * count.white_queens;

  centipawns_t black_score =
      PAWN_VAL * count.black_pawns + KNIGHT_VAL * count.black_knights +
      BISHOP_VAL * count.black_bishops + ROOK_VAL * count.black_rooks +
      QUEEN_VAL * count.black_queens;

  return white_score - black_score;
}

} // namespace siegbert