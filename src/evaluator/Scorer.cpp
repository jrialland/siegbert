#include "evaluator/Scorer.hpp"

namespace siegbert {


int Scorer::getScore(BoardState& bs) {
    PiecesCount count = bs.count_pieces();
    int white = count.white_pawns + count.white_bishops * 3 + count.white_knights * 3 + count.white_rooks * 5 + count.white_queens * 10;
    int black = count.black_pawns + count.black_bishops * 3 + count.black_knights * 3 + count.black_rooks * 5 + count.black_queens * 10;
    return white - black;
}


}