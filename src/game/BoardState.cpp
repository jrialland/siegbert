
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "game/BoardState.hpp"

namespace siegbert {

Move::Move() {}

Move::Move(const move_t &m) : move_t(m) {}

std::string Move::to_str() const {
  std::ostringstream ss;
  ss << "abcdefgh"[COL(from)];
  ss << "12345678"[ROW(from)];
  ss << "abcdefgh"[COL(to)];
  ss << "12345678"[ROW(to)];
  return ss.str();
}

std::string Move::to_json() const {
  std::ostringstream ss;
  ss << "{";
  ss << "\"piece\":\"" << piece << "\", ";
  if (captured) {
    ss << "\"captured\":\"" << captured << "\", ";
  }
  if (promotion) {
    ss << "\"promotion\":\"" << promotion << "\", ";
  }
  if (pawn_jumstart) {
    ss << "\"pawn_jumstart\":true, ";
  }
  if (kingside_castling) {
    ss << "\"kingside_castling\":true, ";
  }
  if (queenside_castling) {
    ss << "\"queenside_castling\":true, ";
  }
  if (enpassant) {
    ss << "\"enpassant\":true, ";
  }
  ss << "\"from\":\""
     << "abcdefgh"[COL(from)] << "12345678"[ROW(from)] << "\", ";
  ss << "\"to\":\""
     << "abcdefgh"[COL(to)] << "12345678"[ROW(to)] << "\"";
  ss << "}";
  return ss.str();
}

std::ostream &operator<<(std::ostream &os, const Move &move) {
  os << move.to_str();
  return os;
}

BoardState::BoardState() {}

BoardState BoardState::initial() {
  BoardState self;
  boardstate_initial(&self);
  return self;
}

BoardState BoardState::from_fen(const std::string &fen) {
  BoardState self;
  boardstate_from_fen(&self, fen.c_str());
  return self;
}

std::string BoardState::to_fen() const {
  char tmp[FEN_STR_SIZE];
  return boardstate_to_fen(this, tmp);
}

std::string BoardState::to_str() const {
  char tmp[BOARDSTATE_STR_SIZE];
  return boardstate_to_str(this, tmp);
}

char BoardState::piece_at(square_t square) const {
  return boardstate_piece_at(this, square);
}

std::vector<Move> BoardState::generate_moves() const {
  std::vector<Move> moves;
  move_t tmp[MOVES_MAX] = {0};
  boardstate_generate_moves(this, tmp);
  move_t *current_move = tmp;
  while (current_move->piece) {
    moves.push_back(Move(*current_move));
    current_move += 1;
  }
  return moves;
}

Move BoardState::get_move(const std::string &san) const {
  Move m;
  if (boardstate_get_move(this, san.c_str(), &m)) {
    return m;
  } else {
    std::ostringstream ss;
    ss << "illegal move : '" << san << "'";
    throw std::invalid_argument(ss.str());
  }
}

bool BoardState::make_move(const Move &move) {
  return boardstate_make_move(this, &move);
}

boardstate_memento_t BoardState::memento() const {
  boardstate_memento_t memento;
  boardstate_get_memento(this, &memento);
  return memento;
}

void BoardState::unmake_move(const Move &move,
                             const boardstate_memento_t &memento) {
  boardstate_unmake_move(this, &move, &memento);
}

std::string BoardState::attacked_str() const {
  char tmp[73];
  bboard_to_str(boardstate_attacked(this), tmp);
  return tmp;
}

void BoardState::set_white_to_move(bool val) { white_to_move = val; }

bool BoardState::is_white_to_move() const { return white_to_move; }

int BoardState::get_halfmoves() const { return halfmoves; }

uint64_t BoardState::get_enpassant() const { return enpassant; }

bool BoardState::is_check() const {
  return boardstate_is_check(this);
}

pieces_count_t BoardState::count_pieces() const {
  pieces_count_t pc;
  boardstate_count_pieces(this, &pc);
  return pc;
}

uint64_t BoardState::get_zobrist_hash() const { return z; }

std::ostream &operator<<(std::ostream &os, const BoardState &boardstate) {
  os << boardstate.to_str();
  return os;
}

} // namespace siegbert
