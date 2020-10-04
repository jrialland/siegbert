
#ifndef BoardState_HPP
#define BoardState_HPP

#include <string>
#include <vector>

typedef uint8_t square_t;

#define SQUARE(R, C) ((square_t)(16 * (R) + (C)))
#define ROW(S) (((square_t)(S)) >> 4)
#define COL(S) (((square_t)(S)) & 7)
#define OFFSET(S) ((S + (S & 7)) >> 1)
#define BBOARD(S) (((uint64_t)1) << OFFSET(S))
#define NAME(S) (std::string({(char)('a'+COL(S))})+std::to_string(1+ROW(S)))
namespace siegbert {

struct Move {

public:
  Move();

  square_t from;
  square_t to;
  char piece;
  char captured;
  char promotion;
  bool enpassant;
  bool kingside_castling;
  bool queenside_castling;
  bool pawn_jumstart;
  int weight;
  
  std::string to_str() const;

  std::string to_json() const;

  uint16_t to_polyglot(bool white) const; 

};

std::ostream &operator<<(std::ostream &os, const Move &move);

struct Castling {
public:
  Castling();
  bool kingside;
  bool queenside;
};

struct Memento {
  uint64_t z;
  uint64_t enpassant;
  int halfmoves;
  Castling castling;
};

struct PiecesCount {
  int white_knights;
  int white_bishops;
  int white_rooks;
  int white_queens;
  int white_pawns;
  int black_knights;
  int black_bishops;
  int black_rooks;
  int black_queens;
  int black_pawns;
};

struct Piece {
  Piece();
  char name;
  square_t square;
};

struct StateUpdateResult {
  Castling castling_rights;
  uint64_t enpassant;
  StateUpdateResult(const Castling &castling);
};

class State {
private:
  void remove_from_piece_list(square_t square);

public:
  State();
  uint64_t presence;
  uint64_t pawns;
  uint64_t rooks;
  uint64_t knights;
  uint64_t bishops;
  uint64_t king;
  Piece pieces[17];
  int npieces;

  void enplace(char piece, square_t square);

  uint64_t compute_attack(const State &opponent, bool im_white) const;

  void move_piece(char piece, square_t from, square_t to);

  void remove_piece(square_t square);

  std::string to_str() const;

  char piece_at(square_t square) const;

  void update_for_capture(const Move &move, bool white);

  void update_for_move(const Move &move, bool white, StateUpdateResult &result);
};

struct BoardState {

private:
  BoardState();

  void evolve_z(const Move &move, const Castling &previous_castling,
                int previous_halfmoves, uint64_t previous_enpassant);

  void recompute_z();

public:
  uint64_t z; /* zobrist hash */
  uint64_t enpassant;
  Castling white_castling;
  Castling black_castling;
  int halfmoves;
  int moves;
  State white;
  State black;
  bool white_to_move;

  static BoardState initial();

  static BoardState from_fen(const std::string &fen);

  std::string to_fen() const;

  std::string to_str() const;

  char piece_at(square_t square) const;

  std::vector<Move> generate_moves() const;

  Move get_move(const std::string &san) const;

  bool make_move(const Move &move);

  Memento memento() const;

  void unmake_move(const Move &move, const Memento &memento);

  bool is_check() const;

  std::string attacked_str() const;

  void set_white_to_move(bool val);

  bool is_white_to_move() const;

  int get_halfmoves() const;

  uint64_t get_enpassant() const;

  PiecesCount count_pieces() const;

  uint64_t get_zobrist_hash() const;
};

std::ostream &operator<<(std::ostream &os, const BoardState &boardstate);

uint8_t square_for_bboard(uint64_t l);

} // namespace siegbert
#endif
