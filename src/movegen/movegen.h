#ifndef MOVEGEN_H
#define MOVEGEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t square_t;
#define SQUARE(R, C) ((square_t)(16 * (R) + (C)))
#define ROW(S) (((square_t)(S)) >> 4)
#define COL(S) (((square_t)(S)) & 7)
#define OFFSET(S) ((S + (S & 7)) >> 1)
#define BBOARD(S) (((uint64_t)1) << OFFSET(S))

#define STATE_MAX_PIECES 17
#define MOVES_MAX 256
#define BOARDSTATE_STR_SIZE (64 + 7 + 1)
#define FEN_STR_SIZE 128

typedef struct _piece {
  char name;
  square_t square;
} piece_t;

typedef struct _move {
  square_t from;
  square_t to;
  char piece;
  char captured;
  char promotion;
  bool enpassant;
  bool kingside_castling;
  bool queenside_castling;
  bool pawn_jumstart;
} move_t;

typedef struct _castling_t {
  bool kingside;
  bool queenside;
} castling_t;

typedef struct _state {
  uint64_t presence;
  uint64_t pawns;
  uint64_t rooks;
  uint64_t knights;
  uint64_t bishops;
  uint64_t king;
  piece_t pieces[STATE_MAX_PIECES];
  int npieces;
} state_t;

typedef struct _boardstate {
  uint64_t z; /* zobrist hash */
  uint64_t enpassant;
  castling_t white_castling;
  castling_t black_castling;
  int halfmoves;
  int moves;
  state_t white;
  state_t black;
  bool white_to_move;
} boardstate_t;

typedef struct _boardstate_memento {
  uint64_t z;
  uint64_t enpassant;
  int halfmoves;
  castling_t castling;
} boardstate_memento_t;

typedef struct _pieces_count {
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
} pieces_count_t;

square_t square_for_bboard(uint64_t l);

char *bboard_to_str(uint64_t bboard, char *buf);

boardstate_t *boardstate_initial(boardstate_t *boardstate);

char boardstate_piece_at(const boardstate_t *boardstate, const square_t square);

char *boardstate_to_str(const boardstate_t *boardstate, char *buf);

boardstate_t *boardstate_from_fen(boardstate_t *boardstate, const char *fen);

char *boardstate_to_fen(const boardstate_t *boardstate, char *fen);

int boardstate_generate_moves(const boardstate_t *boardstate, move_t *buffer);

move_t *boardstate_get_move(const boardstate_t *boardstate, const char *san,
                            move_t *move);

void boardstate_get_memento(const boardstate_t *boardstate,
                            boardstate_memento_t *memento);

bool boardstate_make_move(boardstate_t *boardstate, const move_t *move);

void boardstate_unmake_move(boardstate_t *boardstate, const move_t *move,
                            const boardstate_memento_t *memento);

bool boardstate_is_check(const boardstate_t *boardstate);

uint64_t boardstate_attacked(const boardstate_t *boardstate);

void boardstate_count_pieces(const boardstate_t *boardstate,
                             pieces_count_t *pc);

#ifdef MOVEGEN_PRIVATE
uint64_t boardstate_recompute_zobrist_hash(boardstate_t *boardstate);

void boardstate_evolve_zobrist_hash(boardstate_t *boardstate,
                                    const boardstate_memento_t *previous_values,
                                    const move_t *move);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif
