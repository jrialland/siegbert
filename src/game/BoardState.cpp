
#include <libpopcnt.h>

#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
using namespace std;

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
using namespace boost;

#include "game/BoardState.hpp"
#include "game/BoardState_constants.hpp"

namespace siegbert {

const string abcdefgh = "abcdefgh";

Castling::Castling() : kingside(false), queenside(false) {}

Piece::Piece() : name('\0'), square(0) {}

State::State()
    : presence(0), pawns(0), rooks(0), knights(0), bishops(0), king(0),
      npieces(0) {}

Move::Move()
    : from(0), to(0), piece(0), captured(0), promotion(0), enpassant(0),
      kingside_castling(false), queenside_castling(false),
      pawn_jumstart(false) {}

////////////////////////////////////////////////////////////////////////////////
string Move::to_str() const {
  ostringstream ss;
  ss << "abcdefgh"[COL(from)];
  ss << "12345678"[ROW(from)];
  ss << "abcdefgh"[COL(to)];
  ss << "12345678"[ROW(to)];
  if(promotion) {
    ss << promotion;
  }
  return ss.str();
}

////////////////////////////////////////////////////////////////////////////////
string Move::to_json() const {
  ostringstream ss;
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

////////////////////////////////////////////////////////////////////////////////
ostream &operator<<(ostream &os, const Move &move) {
  os << move.to_str();
  return os;
}

////////////////////////////////////////////////////////////////////////////////
void State::enplace(char p, square_t square) {
  uint64_t bboard = BBOARD(square);
  presence |= bboard;
  switch (p) {
  case 'p':
    pawns |= bboard;
    break;
  case 'r':
    rooks |= bboard;
    break;
  case 'b':
    bishops |= bboard;
    break;
  case 'q':
    rooks |= bboard;
    bishops |= bboard;
    break;
  case 'n':
    knights |= bboard;
    break;
  case 'k':
    king = bboard;
    break;
  }
  pieces[npieces].name = p;
  pieces[npieces].square = square;
  npieces += 1;
}

////////////////////////////////////////////////////////////////////////////////
string State::to_str() const {
  string s;
  s.reserve(72);
  int i = 0;
  for (int row = 7; row >= 0; row -= 1) {
    if (row < 7) {
      s += '\n';
    }
    for (int col = 0; col < 8; col += 1) {
      char p = piece_at(SQUARE(row, col));
      s += p ? p : '.';
    }
  }
  return s;
}

////////////////////////////////////////////////////////////////////////////////
char State::piece_at(square_t square) const {
  uint64_t bboard = BBOARD(square);
  if (bboard & presence) {
    if (bboard & pawns) {
      return 'p';
    }
    if (bboard & rooks) {
      return bboard & bishops ? 'q' : 'r';
    }
    if (bboard & bishops) {
      return bboard & rooks ? 'q' : 'b';
    }
    if (bboard & knights) {
      return 'n';
    }
    if (bboard == king) {
      return 'k';
    }
  }
  return '\0';
}

////////////////////////////////////////////////////////////////////////////////
void State::remove_from_piece_list(square_t square) {
  Piece *p = pieces;
  for (int i = 0; i < npieces; i += 1) {
    /* find the item to be removed */
    if (pieces[i].square == square) {
      /* decrement piece count */
      npieces -= 1;
      /* if it was not the last item */
      if (i < npieces) {
        /* copy last item */
        pieces[i].name = pieces[npieces].name;
        pieces[i].square = pieces[npieces].square;
      }
      /* remove the last item */
      pieces[npieces].name = '\0';
      break;
    }
  }
}

template <const bboard_and_square_t ray[64][8]>
inline uint64_t ray_attack(const uint64_t occupied, const int offset) {
  uint64_t a = 0;
  auto ptr = ray[offset];
  while (ptr->bboard) {
    a = a | ptr->bboard;
    if (ptr->bboard & occupied) {
      break;
    }
    ptr += 1;
  }
  return a;
}

////////////////////////////////////////////////////////////////////////////////
uint64_t State::compute_attack(const State &opponent, bool im_white) const {

  const uint64_t occupied = presence | opponent.presence;
  uint64_t a = 0;
  const Piece *p = pieces;

  do {
    int offset = OFFSET(p->square);
    switch (p->name) {
    case 'p':
      a = a | (im_white ? WHITE_PAWN_CAPTURES : BLACK_PAWN_CAPTURES)[offset];
      break;
    case 'n':
      a = a | KNIGHT_CAPTURES[offset];
      break;
    case 'k':
      a = a | KING_CAPTURES[offset];
      break;
    case 'r':
      a = a | ray_attack<ROOK_RAY_N>(occupied, offset);
      a = a | ray_attack<ROOK_RAY_E>(occupied, offset);
      a = a | ray_attack<ROOK_RAY_S>(occupied, offset);
      a = a | ray_attack<ROOK_RAY_W>(occupied, offset);
      break;
    case 'q':
      a = a | ray_attack<ROOK_RAY_N>(occupied, offset);
      a = a | ray_attack<ROOK_RAY_E>(occupied, offset);
      a = a | ray_attack<ROOK_RAY_S>(occupied, offset);
      a = a | ray_attack<ROOK_RAY_W>(occupied, offset);
      /*nobreak;*/
    case 'b':
      a = a | ray_attack<BISHOP_RAY_NE>(occupied, offset);
      a = a | ray_attack<BISHOP_RAY_SE>(occupied, offset);
      a = a | ray_attack<BISHOP_RAY_NW>(occupied, offset);
      a = a | ray_attack<BISHOP_RAY_SW>(occupied, offset);
      break;
    }
    p += 1;
  } while (p->name);
  return a;
}

////////////////////////////////////////////////////////////////////////////////
void State::move_piece(char piece, square_t from, square_t to) {
  Piece *p = pieces;
  do {
    if (p->name == piece && p->square == from) {
      p->square = to;
      const uint64_t mask = ~BBOARD(from);
      const uint64_t bboard_to = BBOARD(to);
      presence = (presence & mask) | bboard_to;
      switch (piece) {
      case 'r':
        rooks = (rooks & mask) | bboard_to;
        break;
      case 'n':
        knights = (knights & mask) | bboard_to;
        break;
      case 'q':
        rooks = (rooks & mask) | bboard_to; /*nobreak;*/
      case 'b':
        bishops = (bishops & mask) | bboard_to;
        break;
      case 'k':
        king = bboard_to;
        break;
      case 'p':
        pawns = (pawns & mask) | bboard_to;
        break;
      }
      return;
    }
    p += 1;
  } while (p->name);
}

////////////////////////////////////////////////////////////////////////////////
void State::remove_piece(square_t square) {
  const uint64_t mask = ~BBOARD(square);
  pawns &= mask;
  rooks &= mask;
  knights &= mask;
  bishops &= mask;
  king &= mask;
  presence &= mask;
  remove_from_piece_list(square);
}

////////////////////////////////////////////////////////////////////////////////
void State::update_for_capture(const Move &move, bool white) {
  if (move.enpassant) {
    int col = COL(move.to);
    int capture_row = white ? ROW(move.to) + 1 : ROW(move.to) - 1;
    square_t dest = SQUARE(capture_row, col);
    const uint64_t mask = ~BBOARD(dest);
    presence &= mask;
    pawns &= mask;
    remove_from_piece_list(dest);
  } else {
    remove_piece(move.to);
  }
}

////////////////////////////////////////////////////////////////////////////////
void State::update_for_move(const Move &move, bool white,
                            StateUpdateResult &result) {
  /* move the rooks when castling */
  if (move.kingside_castling) {
    const int castling_row = white ? 0 : 7;
    move_piece('r', SQUARE(castling_row, 7), SQUARE(castling_row, 5));
  } else if (move.queenside_castling) {
    const int castling_row = white ? 0 : 7;
    move_piece('r', SQUARE(castling_row, 0), SQUARE(castling_row, 3));
  }

  /* update castling rights */
  if (move.piece == 'k') {
    result.castling_rights.kingside = false;
    result.castling_rights.queenside = false;
  } else if (move.piece == 'r') {
    int from_row = ROW(move.from);
    if (from_row == (white ? 0 : 7)) {
      int from_col = COL(move.from);
      if (from_col == 0) {
        result.castling_rights.queenside = false;
      } else if (from_col == 7) {
        result.castling_rights.kingside = false;
      }
    }
  }

  /* move the piece */
  if (move.promotion) {
    remove_piece(move.from);
    enplace(move.promotion, move.to);
  } else {
    move_piece(move.piece, move.from, move.to);
  }

  /* set enpassant if applicable */
  if (move.pawn_jumstart) {
    result.enpassant = BBOARD(SQUARE(white ? 2 : 5, COL(move.from)));
  } else {
    result.enpassant = 0;
  }
}

BoardState::BoardState()
    : z(0), enpassant(0), halfmoves(0), moves(0), white_to_move(0) {}

////////////////////////////////////////////////////////////////////////////////
BoardState BoardState::initial() {
  return from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

////////////////////////////////////////////////////////////////////////////////
BoardState BoardState::from_fen(const string &fen) {
  BoardState self;
  vector<string> items;
  State *current;

  boost::split(items, fen, boost::is_space());
  if (items.size() != 6) {
    throw invalid_argument("invalid fen");
  }

  /* positions */
  string positions = items.at(0);
  int row = 7;
  int col = 0;
  for (int i = 0, len = positions.length(); i < len; i += 1) {
    char c = positions[i];
    if (c == '/') {
      row = row - 1;
      col = 0;
      continue;
    }
    if (c >= '0' && c <= '8') {
      col = col + (c - '0');
      continue;
    }
    char piece = tolower(c);
    current = piece == c ? &self.black : &self.white;
    current->enplace(piece, SQUARE(row, col));
    col += 1;
  }

  /* turn */
  string turn = items[1];
  self.white_to_move = turn[0] == 'w';

  /* moves */
  string moves = items[5];
  self.moves = stoi(moves);

  /* castlings */
  string castlings = items[2];
  self.white_castling.kingside = castlings.find('K') != string::npos;
  self.white_castling.queenside = castlings.find('Q') != string::npos;
  self.black_castling.kingside = castlings.find('k') != string::npos;
  self.black_castling.queenside = castlings.find('q') != string::npos;

  /* enpassant */
  string enpassant = items[3];
  if (enpassant[0] == '-') {
    self.enpassant = 0;
  } else {
    int col = enpassant[0] - 'a';
    int row = enpassant[1] - '1';
    self.enpassant = BBOARD(SQUARE(row, col));
  }

  /* halfmoves */
  string halfmoves = items[4];
  self.halfmoves = stoi(halfmoves);

  self.recompute_z();
  return self;
}

////////////////////////////////////////////////////////////////////////////////
string BoardState::to_fen() const {
  string fen;
  fen.reserve(128);
  /* position */
  for (int row = 7; row >= 0; row -= 1) {
    if (row < 7) {
      fen += '/';
    }
    int empty = 0;
    for (int col = 0; col < 8; col += 1) {
      char p = piece_at(SQUARE(row, col));
      if (p == '\0') {
        empty += 1;
      } else {
        if (empty > 0) {
          fen += to_string(empty);
          empty = 0;
        }
        fen += p;
      }
    }
    if (empty > 0) {
      fen += to_string(empty);
    }
  }

  /* current player */
  fen += ' ';
  fen += white_to_move ? 'w' : 'b';

  /* castlings */
  fen += ' ';
  bool has_castlings = false;
  if (white_castling.kingside) {
    fen += 'K';
    has_castlings = true;
  }
  if (white_castling.queenside) {
    fen += 'Q';
    has_castlings = true;
  }
  if (black_castling.kingside) {
    fen += 'k';
    has_castlings = true;
  }
  if (black_castling.queenside) {
    fen += 'q';
    has_castlings = true;
  }
  if (!has_castlings) {
    fen += '-';
  }

  /* enpassant */
  fen += ' ';
  if (enpassant) {
    square_t e = square_for_bboard(enpassant);
    fen += abcdefgh[COL(e)];
    fen += to_string(1 + ROW(e));
  } else {
    fen += '-';
  }

  /* half moves */
  fen += ' ';
  fen += to_string(halfmoves);

  /* moves */
  fen += ' ';
  fen += to_string(moves);

  return fen;
}

////////////////////////////////////////////////////////////////////////////////
string BoardState::to_str() const {
  string s;
  const string s_white = white.to_str();
  const string s_black = black.to_str();
  s.reserve(72);
  for (int i = 0, len = s_white.length(); i < len; i++) {
    if (s_white[i] == '.') {
      s += s_black[i];
    } else {
      s += toupper(s_white[i]);
    }
  }
  return s;
}

////////////////////////////////////////////////////////////////////////////////
char BoardState::piece_at(square_t square) const {
  const char p = white.piece_at(square);
  if (p != '\0') {
    return toupper(p);
  } else {
    return black.piece_at(square);
  }
}

////////////////////////////////////////////////////////////////////////////////
StateUpdateResult::StateUpdateResult(const Castling &castling)
    : castling_rights(castling), enpassant(0) {}

////////////////////////////////////////////////////////////////////////////////
bool BoardState::make_move(const Move &move) {
  State *moving;
  State *opponent;
  Castling *castling;
  bool opponent_is_white;

  if (white_to_move) {
    moving = &white;
    opponent = &black;
    castling = &white_castling;
    opponent_is_white = false;
  } else {
    moving = &black;
    opponent = &white;
    castling = &black_castling;
    opponent_is_white = true;
  }

  /* save the states if we have to revert later */
  const State saved_white = white;
  const State saved_black = black;

  /* update the state for the moving side */
  StateUpdateResult moveresult(*castling);
  moving->update_for_move(move, !opponent_is_white, moveresult);

  /* update the opponents pieces if this is a capture */
  if (move.captured) {
    opponent->update_for_capture(move, opponent_is_white);
  }

  /* checks which squares are attacked now */
  uint64_t attacked = opponent->compute_attack(*moving, opponent_is_white);
  if (moving->king & attacked) {

    // Oops, the move is illegal, we gotta to revert
    white = saved_white;
    black = saved_black;
    return false;

  } else {

    Castling previous_castling;
    int previous_halfmoves = halfmoves;
    uint64_t previous_enpassant = enpassant;

    enpassant = moveresult.enpassant;
    if (white_to_move) {
      previous_castling = white_castling;
      white_castling = moveresult.castling_rights;
      white_to_move = false;
    } else {
      previous_castling = black_castling;
      black_castling = moveresult.castling_rights;
      white_to_move = true;
      moves += 1;
    }

    if (move.piece == 'p' || move.captured) {
      halfmoves = 0;
    } else {
      halfmoves += 1;
    }

    evolve_z(move, previous_castling, previous_halfmoves, previous_enpassant);

    return true;
  }
}

////////////////////////////////////////////////////////////////////////////////
Memento BoardState::memento() const {
  Memento m;
  m.castling = white_to_move ? white_castling : black_castling;
  m.halfmoves = halfmoves;
  m.enpassant = enpassant;
  m.z = z;
  return m;
}

////////////////////////////////////////////////////////////////////////////////
void BoardState::unmake_move(const Move &move, const Memento &memento) {
  State *hasplayed;
  State *opponent;
  Castling *castling;

  if (white_to_move) {
    hasplayed = &black;
    opponent = &white;
    castling = &black_castling;
    moves -= 1;
  } else {
    hasplayed = &white;
    opponent = &black;
    castling = &white_castling;
  }

  if (move.captured) {
    if (move.enpassant) {
      int row = hasplayed == &white ? 4 : 3;
      opponent->enplace('p', SQUARE(row, COL(move.to)));
    } else {
      opponent->enplace(move.captured, move.to);
    }
  }

  if (move.promotion) {
    hasplayed->remove_piece(move.to);
    hasplayed->enplace('p', move.from);
  } else {
    hasplayed->move_piece(move.piece, move.to, move.from);
  }

  if (move.kingside_castling) {
    int row = hasplayed == &white ? 0 : 7;
    hasplayed->move_piece('r', SQUARE(row, 5), SQUARE(row, 7));
  } else if (move.queenside_castling) {
    int row = hasplayed == &white ? 0 : 7;
    hasplayed->move_piece('r', SQUARE(row, 3), SQUARE(row, 0));
  }

  if (white_to_move) {
    white_to_move = false;
    black_castling = memento.castling;
  } else {
    white_to_move = true;
    white_castling = memento.castling;
  }

  halfmoves = memento.halfmoves;
  enpassant = memento.enpassant;
  z = memento.z;
}

////////////////////////////////////////////////////////////////////////////////
void BoardState::set_white_to_move(bool val) { white_to_move = val; }

////////////////////////////////////////////////////////////////////////////////
bool BoardState::is_white_to_move() const { return white_to_move; }

////////////////////////////////////////////////////////////////////////////////
int BoardState::get_halfmoves() const { return halfmoves; }

////////////////////////////////////////////////////////////////////////////////
uint64_t BoardState::get_enpassant() const { return enpassant; }

////////////////////////////////////////////////////////////////////////////////
bool BoardState::is_check() const {
  if (white_to_move) {
    uint64_t attacked = black.compute_attack(white, false);
    return white.king & attacked;
  } else {
    uint64_t attacked = white.compute_attack(black, true);
    return black.king & attacked;
  }
}

////////////////////////////////////////////////////////////////////////////////
PiecesCount BoardState::count_pieces() const {
  PiecesCount pc;
  pc.white_knights = popcnt64(white.knights);
  pc.white_pawns = popcnt64(white.pawns);
  pc.white_queens = popcnt64(white.rooks & white.bishops);
  pc.white_rooks = popcnt64(white.rooks) - pc.white_queens;
  pc.white_bishops = popcnt64(white.knights) - pc.white_queens;

  pc.black_knights = popcnt64(black.knights);
  pc.black_pawns = popcnt64(black.pawns);
  pc.black_queens = popcnt64(black.rooks & black.bishops);
  pc.black_rooks = popcnt64(black.rooks) - pc.black_queens;
  pc.black_bishops = popcnt64(black.knights) - pc.black_queens;
  return pc;
}

////////////////////////////////////////////////////////////////////////////////
uint64_t BoardState::get_zobrist_hash() const { return z; }

////////////////////////////////////////////////////////////////////////////////
ostream &operator<<(ostream &os, const BoardState &boardstate) {
  os << boardstate.to_str();
  return os;
}

template <const bboard_and_square_t ray[64][8]>
inline void branch_move(vector<Move> &moves, const Piece *piece,
                        const State *moving, const State *opponent,
                        const uint64_t opponent_capturable) {
  const bboard_and_square_t *r = ray[OFFSET(piece->square)];
  while (r->bboard) {
    if (r->bboard & moving->presence)
      break;
    Move move;
    move.piece = piece->name;
    move.from = piece->square;
    move.to = r->square;
    move.captured =
        r->bboard & opponent_capturable ? opponent->piece_at(move.to) : '\0';
    moves.push_back(move);
    if (move.captured)
      break;
    r += 1;
  }
}

template <const bboard_and_square_t ray0[64][8],
          const bboard_and_square_t ray1[64][8],
          const bboard_and_square_t ray2[64][8],
          const bboard_and_square_t ray3[64][8]>
inline void ray_moves(vector<Move> &moves, const Piece *piece,
                      const State *moving, const State *opponent,
                      const uint64_t opponent_capturable) {
  branch_move<ray0>(moves, piece, moving, opponent, opponent_capturable);
  branch_move<ray1>(moves, piece, moving, opponent, opponent_capturable);
  branch_move<ray2>(moves, piece, moving, opponent, opponent_capturable);
  branch_move<ray3>(moves, piece, moving, opponent, opponent_capturable);
}

template <int pawn_direction, int dir>
inline void pawn_capture(vector<Move> &moves, const Piece *piece,
                         const State *opponent,
                         const uint64_t opponent_capturable,
                         const uint64_t enpassant, const int dest_row) {
  const square_t dest_square =
      SQUARE(ROW(piece->square) + pawn_direction, COL(piece->square) + dir);
  const uint64_t dest = BBOARD(dest_square);
  if (dest & (opponent_capturable | enpassant)) {
    if (dest_row != (pawn_direction == 1 ? 7 : 0)) {
      Move move;
      move.piece = 'p';
      move.from = piece->square;
      move.to = dest_square;
      if (move.enpassant = dest == enpassant) {
        move.captured = 'p';
      } else {
        move.captured = opponent->piece_at(dest_square);
      }
      moves.push_back(move);
    } else {
      char captured = opponent->piece_at(dest_square);
      for (int i = 0; i < 4; i += 1) {
        Move move;
        move.piece = 'p';
        move.from = piece->square;
        move.to = dest_square;
        move.captured = captured;
        move.promotion = "qnrb"[i];
        moves.push_back(move);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
vector<Move> BoardState::generate_moves() const {

  vector<Move> moves;
  const State *moving;
  const State *opponent;
  bool opponent_is_white;

  int count = 0;
  int initial_row;
  int pawn_row;
  int promotion_row;
  int pawn_direction;

  const Castling *castling = &white_castling;
  if (white_to_move) {
    castling = &white_castling;
    moving = &white;
    opponent = &black;
    opponent_is_white = false;
    initial_row = 0;
    pawn_row = 1;
    promotion_row = 7;
    pawn_direction = 1;
  } else {
    castling = &black_castling;
    moving = &black;
    opponent = &white;
    opponent_is_white = true;
    initial_row = 7;
    pawn_row = 6;
    promotion_row = 0;
    pawn_direction = -1;
  }

  const uint64_t opponent_capturable = opponent->presence & ~(opponent->king);
  const uint64_t occupied = moving->presence | opponent->presence;

  const Piece *p = moving->pieces;

  do {

    int from_row = ROW(p->square), from_col = COL(p->square);
    int from_offset = OFFSET(p->square);
    switch (p->name) {

    case 'n': {
      const bboard_and_square_t *s = KNIGHT_MOVES[from_offset];
      for (int i = 0; i < 8 && s->bboard; i += 1) {
        if (s->bboard & ~moving->presence) {
          Move move;
          move.from = p->square;
          move.to = s->square;
          move.piece = 'n';
          move.captured = s->bboard & opponent_capturable
                              ? opponent->piece_at(move.to)
                              : '\0';
          moves.push_back(move);
        }
        s += 1;
      }
    } break;

    case 'k': {

      uint64_t attacked = opponent->compute_attack(*moving, opponent_is_white);
      const bboard_and_square_t *s = KING_MOVES[from_offset];
      const uint64_t notok_squares = moving->presence | attacked;

      for (int i = 0; i < 8 && s->bboard; i += 1) {
        if (s->bboard & ~notok_squares) {
          Move move;
          move.piece = 'k';
          move.from = p->square;
          move.to = s->square;
          move.captured = s->bboard & opponent_capturable
                              ? opponent->piece_at(move.to)
                              : '\0';
          moves.push_back(move);
        }
        s += 1;
      }

      /* castling */
      if (BBOARD(p->square) & ~attacked) {

#define OCCUPIED_OR_ATTACKED(col)                                              \
  (BBOARD(SQUARE(initial_row, col)) & (opponent->presence | notok_squares))

#define OCCUPIED(col)                                                          \
  (BBOARD(SQUARE(initial_row, col)) & (moving->presence | opponent->presence))

        if (castling->kingside &&
            !(OCCUPIED_OR_ATTACKED(5) || OCCUPIED_OR_ATTACKED(6))) {

          Move move;
          move.piece = 'k';
          move.from = SQUARE(initial_row, 4);
          move.to = SQUARE(initial_row, 6);
          move.kingside_castling = true;
          moves.push_back(move);
        }

        if (castling->queenside && !(OCCUPIED_OR_ATTACKED(3) ||
                                     OCCUPIED_OR_ATTACKED(2) || OCCUPIED(1))) {
          Move move;
          move.piece = 'k';
          move.from = SQUARE(initial_row, 4);
          move.to = SQUARE(initial_row, 2);
          move.queenside_castling = true;
          moves.push_back(move);
        }

#undef OCCUPIED_OR_ATTACKED
#undef OCCUPIED
      }
    } break;

    case 'r': {
      ray_moves<ROOK_RAY_N, ROOK_RAY_E, ROOK_RAY_S, ROOK_RAY_W>(
          moves, p, moving, opponent, opponent_capturable);
    } break;
    case 'b': {
      ray_moves<BISHOP_RAY_NE, BISHOP_RAY_NW, BISHOP_RAY_SE, BISHOP_RAY_SW>(
          moves, p, moving, opponent, opponent_capturable);
    } break;
    case 'q': {
      ray_moves<ROOK_RAY_N, ROOK_RAY_E, ROOK_RAY_S, ROOK_RAY_W>(
          moves, p, moving, opponent, opponent_capturable);
      ray_moves<BISHOP_RAY_NE, BISHOP_RAY_NW, BISHOP_RAY_SE, BISHOP_RAY_SW>(
          moves, p, moving, opponent, opponent_capturable);
    } break;
    case 'p': {

      int dest_row = from_row + pawn_direction;
      square_t dest_square = SQUARE(dest_row, from_col);
      uint64_t dest = BBOARD(dest_square);

      /*non capture moves*/
      if (dest & ~occupied) {

        /* one step forward */
        if (dest_row != promotion_row) {
          Move move;
          move.piece = 'p';
          move.from = p->square;
          move.to = dest_square;
          moves.push_back(move);
        } else {
          /* promotion */
          for (int i = 0; i < 4; i += 1) {
            Move move;
            move.piece = 'p';
            move.promotion = "qnrb"[i];
            move.from = p->square;
            move.to = dest_square;
            moves.push_back(move);
          }
        }

        /* initial move : 2 steps forward */
        if (from_row == pawn_row) {
          int jump_dest_row = from_row + pawn_direction * 2;
          square_t jump_dest = SQUARE(jump_dest_row, from_col);
          dest = BBOARD(jump_dest);
          if (dest & ~occupied) {
            Move move;
            move.piece = 'p';
            move.pawn_jumstart = true;
            move.from = p->square;
            move.to = jump_dest;
            moves.push_back(move);
          }
        }
      }

      /* capture left */
      if (from_col > 0) {
        if (white_to_move) {
          pawn_capture<1, -1>(moves, p, opponent, opponent_capturable,
                              enpassant, dest_row);
        } else {
          pawn_capture<-1, -1>(moves, p, opponent, opponent_capturable,
                               enpassant, dest_row);
        }
      }

      /* capture right */
      if (from_col < 7) {
        if (white_to_move) {
          pawn_capture<1, 1>(moves, p, opponent, opponent_capturable, enpassant,
                             dest_row);
        } else {
          pawn_capture<-1, 1>(moves, p, opponent, opponent_capturable,
                              enpassant, dest_row);
        }
      }
    } break;
    }

    p += 1;

  } while (p->name);

  return moves;
}

} // namespace siegbert
