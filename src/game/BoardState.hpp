
#ifndef BoardState_HPP
#define BoardState_HPP

#include <string>
#include <vector>

#include "movegen/movegen.h"

//
// C++ wrappers around the movegen structs
//

namespace siegbert {

class BoardState;

class Move : public move_t {

private:
  Move(const move_t &m);

  friend class BoardState;

public:
  Move();

  std::string to_str() const;

  std::string to_json() const;
};

std::ostream &operator<<(std::ostream &os, const Move &move);

class BoardState : private boardstate_t {

private:
  BoardState();

public:
  static BoardState initial();

  static BoardState from_fen(const std::string &fen);

  std::string to_fen() const;

  std::string to_str() const;

  char piece_at(square_t square) const;

  std::vector<Move> generate_moves() const;

  Move get_move(const std::string &san) const;

  bool make_move(const Move &move);

  boardstate_memento_t memento() const;

  void unmake_move(const Move &move, const boardstate_memento_t &memento);

  bool is_check() const;

  std::string attacked_str() const;

  void set_white_to_move(bool val);

  bool is_white_to_move() const;

  int get_halfmoves() const;

  uint64_t get_enpassant() const;

  pieces_count_t count_pieces() const;

  uint64_t get_zobrist_hash() const;
};

std::ostream &operator<<(std::ostream &os, const BoardState &boardstate);

} // namespace siegbert

#endif
