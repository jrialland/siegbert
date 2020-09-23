#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libpopcnt.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#define MOVEGEN_PRIVATE
#include "movegen.h"
#include "movegen_constants.h"
#undef MOVEGEN_PRIVATE

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__)
#define INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define INLINE __forceinline
#else
#define INLINE inline
#endif

static const char *abcdefgh = "abcdefgh";
static const char *_012345678 = "012345678";
static const char *_12345678 = "12345678";

uint8_t square_for_bboard(uint64_t l) {
#if defined(__GNUC__) || defined(__clang__)
  int offset = sizeof(uint64_t) * 8 - 1 - __builtin_clzl(l);
  return 16 * (offset / 8) + (offset % 8);
#elif defined(_MSC_VER)
#pragma intrinsic(_BitScanReverse64)
  unsigned long offset;
  _BitScanReverse64(&offset, l);
  return 16 * (offset / 8) + (offset % 8);
#else
  for (int row = 0; row < 8; row += 1) {
    for (int col = 0; col < 8; col += 1) {
      if (l & (1 << (row * 8 + col))) {
        return 16 * row + col;
      }
    }
  }
  return 0x88;
#endif
}

typedef struct _state_update_result {
  castling_t castling_rights;
  uint64_t enpassant;
} state_update_result_t;

char *bboard_to_str(uint64_t bboard, char *buf) {
  int i = 0;
  for (int row = 7; row >= 0; row -= 1) {
    if (row < 7) {
      buf[i++] = '\n';
    }
    for (int col = 0; col < 8; col += 1) {
      buf[i++] = bboard & BBOARD(SQUARE(row, col)) ? '1' : '0';
    }
  }
  buf[i] = '\0';
  return buf;
}

static INLINE void state_enplace_piece(state_t *state, char piece,
                                       square_t square) {
  uint64_t bboard = BBOARD(square);
  state->presence |= bboard;
  switch (piece) {
  case 'p':
    state->pawns |= bboard;
    break;
  case 'r':
    state->rooks |= bboard;
    break;
  case 'b':
    state->bishops |= bboard;
    break;
  case 'q':
    state->rooks |= bboard;
    state->bishops |= bboard;
    break;
  case 'n':
    state->knights |= bboard;
    break;
  case 'k':
    state->king = bboard;
    break;
  }
  state->pieces[state->npieces].name = piece;
  state->pieces[state->npieces].square = square;
  state->npieces += 1;
}

#define RAY_ATTACK(ray)                                                        \
  do {                                                                         \
    const bboard_and_square_t *ptr = ray[offset];                              \
    while (ptr->bboard) {                                                      \
      a = a | ptr->bboard;                                                     \
      if (ptr->bboard & occupied) {                                            \
        break;                                                                 \
      }                                                                        \
      ptr += 1;                                                                \
    }                                                                          \
  } while (0)

static INLINE uint64_t state_compute_attack(const state_t *state,
                                            const state_t *opponent,
                                            bool attacker_is_white) {
  const uint64_t occupied = state->presence | opponent->presence;
  uint64_t a = 0;
  const piece_t *p = state->pieces;
  do {
    uint8_t offset = OFFSET(p->square);
    switch (p->name) {
    case 'p':
      a = a | (attacker_is_white ? WHITE_PAWN_CAPTURES
                                 : BLACK_PAWN_CAPTURES)[offset];
      break;
    case 'n':
      a = a | KNIGHT_CAPTURES[offset];
      break;
    case 'k':
      a = a | KING_CAPTURES[offset];
      break;
    case 'r':
      RAY_ATTACK(ROOK_RAY_N);
      RAY_ATTACK(ROOK_RAY_E);
      RAY_ATTACK(ROOK_RAY_S);
      RAY_ATTACK(ROOK_RAY_W);
      break;
    case 'q':
      RAY_ATTACK(ROOK_RAY_N);
      RAY_ATTACK(ROOK_RAY_E);
      RAY_ATTACK(ROOK_RAY_S);
      RAY_ATTACK(ROOK_RAY_W);
      /*nobreak;*/
    case 'b':
      RAY_ATTACK(BISHOP_RAY_NE);
      RAY_ATTACK(BISHOP_RAY_SE);
      RAY_ATTACK(BISHOP_RAY_NW);
      RAY_ATTACK(BISHOP_RAY_SW);
      break;
    }
    p += 1;
  } while (p->name);
  return a;
}

#undef RAY_ATTACK

static INLINE void state_move_piece(state_t *state, char piece, square_t from,
                                    square_t to) {
  piece_t *p = state->pieces;
  do {
    if (p->name == piece && p->square == from) {

      /* change location in piece list */
      p->square = to;

      const uint64_t mask = ~BBOARD(from);
      const uint64_t bboard_to = BBOARD(to);

      state->presence = (state->presence & mask) | bboard_to;

      switch (piece) {
      case 'r':
        state->rooks = (state->rooks & mask) | bboard_to;
        break;
      case 'n':
        state->knights = (state->knights & mask) | bboard_to;
        break;
      case 'q':
        state->rooks = (state->rooks & mask) | bboard_to; /*nobreak;*/
      case 'b':
        state->bishops = (state->bishops & mask) | bboard_to;
        break;
      case 'k':
        state->king = bboard_to;
        break;
      case 'p':
        state->pawns = (state->pawns & mask) | bboard_to;
        break;
      }
      return;
    }
    p += 1;
  } while (p->name);
}

static INLINE void state_remove_from_piece_list(state_t *state,
                                                square_t square) {
  piece_t *p = state->pieces;
  for (int i = 0; i < state->npieces; i += 1) {
    /* find the item to be removed */
    if (state->pieces[i].square == square) {
      /* decrement piece count */
      state->npieces -= 1;
      /* if it was not the last item */
      if (i < state->npieces) {
        /* copy last item */
        state->pieces[i].name = state->pieces[state->npieces].name;
        state->pieces[i].square = state->pieces[state->npieces].square;
      }
      /* remove the last item */
      state->pieces[state->npieces].name = '\0';
      break;
    }
  }
}

static INLINE void state_remove_piece(state_t *state, square_t square) {
  const uint64_t mask = ~BBOARD(square);
  state->pawns &= mask;
  state->rooks &= mask;
  state->knights &= mask;
  state->bishops &= mask;
  state->king &= mask;
  state->presence &= mask;
  state_remove_from_piece_list(state, square);
}

static INLINE char state_piece_at(const state_t *state, const square_t square) {
  uint64_t bboard = BBOARD(square);
  if (bboard & state->presence) {
    if (bboard & state->pawns) {
      return 'p';
    }
    if (bboard & state->rooks) {
      return bboard & state->bishops ? 'q' : 'r';
    }
    if (bboard & state->bishops) {
      return bboard & state->rooks ? 'q' : 'b';
    }
    if (bboard & state->knights) {
      return 'n';
    }
    if (bboard == state->king) {
      return 'k';
    }
  }
  return '\0';
}

static INLINE void state_update_for_capture(state_t *state, bool white,
                                            const move_t *move) {
  if (move->enpassant) {
    int col = COL(move->to);
    int capture_row = white ? ROW(move->to) + 1 : ROW(move->to) - 1;
    square_t dest = SQUARE(capture_row, col);
    const uint64_t mask = ~BBOARD(dest);
    state->presence &= mask;
    state->pawns &= mask;
    state_remove_from_piece_list(state, dest);
  } else {
    state_remove_piece(state, move->to);
  }
}

static INLINE void state_update_for_move(state_t *state, bool white,
                                         const move_t *move,
                                         state_update_result_t *result) {

  /* move the rooks when castling */
  if (move->kingside_castling) {
    const int castling_row = white ? 0 : 7;
    state_move_piece(state, 'r', SQUARE(castling_row, 7),
                     SQUARE(castling_row, 5));
  } else if (move->queenside_castling) {
    const int castling_row = white ? 0 : 7;
    state_move_piece(state, 'r', SQUARE(castling_row, 0),
                     SQUARE(castling_row, 3));
  }

  /* update castling rights */
  if (move->piece == 'k') {
    result->castling_rights.kingside = false;
    result->castling_rights.queenside = false;
  } else if (move->piece == 'r') {
    int from_row = ROW(move->from);
    if (from_row == (white ? 0 : 7)) {
      int from_col = COL(move->from);
      if (from_col == 0) {
        result->castling_rights.queenside = false;
      } else if (from_col == 7) {
        result->castling_rights.kingside = false;
      }
    }
  }

  /* move the piece */
  if (move->promotion) {
    state_remove_piece(state, move->from);
    state_enplace_piece(state, move->promotion, move->to);
  } else {
    state_move_piece(state, move->piece, move->from, move->to);
  }

  /* set enpassant if applicable */
  if (move->pawn_jumstart) {
    result->enpassant = BBOARD(SQUARE(white ? 2 : 5, COL(move->from)));
  } else {
    result->enpassant = 0;
  }
}

static INLINE char *state_to_str(const state_t *state, char *buf) {
  int i = 0;
  for (int row = 7; row >= 0; row -= 1) {
    if (row < 7) {
      buf[i++] = '\n';
    }
    for (int col = 0; col < 8; col += 1) {
      char p = state_piece_at(state, SQUARE(row, col));
      buf[i++] = p ? p : '.';
    }
  }
  buf[i] = '\0';
  return buf;
}

boardstate_t *boardstate_initial(boardstate_t *boardstate) {
  return boardstate_from_fen(
      boardstate, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

char *boardstate_to_str(const boardstate_t *boardstate, char *buf) {
  char s_white[BOARDSTATE_STR_SIZE];
  char s_black[BOARDSTATE_STR_SIZE];
  state_to_str(&boardstate->white, s_white);
  state_to_str(&boardstate->black, s_black);
  for (int i = 0; i < BOARDSTATE_STR_SIZE; i++) {
    if (s_white[i] == '.') {
      buf[i] = s_black[i];
    } else {
      buf[i] = toupper(s_white[i]);
    }
  }
  return buf;
}

int boardstate_generate_moves(const boardstate_t *boardstate, move_t *buffer) {

#define RAY_MOVES(piece_name, RAY)                                             \
  {                                                                            \
    const bboard_and_square_t *ray = RAY[from_offset];                         \
    while (ray->bboard) {                                                      \
      if (ray->bboard & moving->presence)                                      \
        break;                                                                 \
      move_t *move = buffer + (count++);                                       \
      move->piece = piece_name;                                                \
      move->from = p->square;                                                  \
      move->to = ray->square;                                                  \
      move->captured = ray->bboard & opponent_capturable                       \
                           ? state_piece_at(opponent, move->to)                \
                           : '\0';                                             \
      if (move->captured)                                                      \
        break;                                                                 \
      ray += 1;                                                                \
    }                                                                          \
  }

#define ROOK_MOVES(piece_name)                                                 \
  RAY_MOVES(piece_name, ROOK_RAY_N);                                           \
  RAY_MOVES(piece_name, ROOK_RAY_E);                                           \
  RAY_MOVES(piece_name, ROOK_RAY_S);                                           \
  RAY_MOVES(piece_name, ROOK_RAY_W);

#define BISHOP_MOVES(piece_name)                                               \
  RAY_MOVES(piece_name, BISHOP_RAY_NE);                                        \
  RAY_MOVES(piece_name, BISHOP_RAY_NW);                                        \
  RAY_MOVES(piece_name, BISHOP_RAY_SE);                                        \
  RAY_MOVES(piece_name, BISHOP_RAY_SW);

  const state_t *moving;
  const state_t *opponent;
  bool opponent_is_white;
  int count = 0, initial_row = 0, pawn_row = 1, promotion_row = 7,
      pawn_direction = 1;
  const castling_t *castling = &boardstate->white_castling;
  if (boardstate->white_to_move) {
    moving = &boardstate->white;
    opponent = &boardstate->black;
    opponent_is_white = false;
  } else {
    moving = &boardstate->black;
    opponent = &boardstate->white;
    opponent_is_white = true;
    initial_row = 7;
    pawn_row = 6;
    promotion_row = 0;
    pawn_direction = -1;
    castling = &boardstate->black_castling;
  }

  const uint64_t opponent_capturable = opponent->presence & ~(opponent->king);
  const uint64_t occupied = moving->presence | opponent->presence;

  const piece_t *p = moving->pieces;

  buffer->piece = '\0';

  do {

    int from_row = ROW(p->square), from_col = COL(p->square);
    int from_offset = OFFSET(p->square);
    switch (p->name) {

    case 'n': {
      const bboard_and_square_t *s = KNIGHT_MOVES[from_offset];
      for (int i = 0; i < 8 && s->bboard; i += 1) {
        if (s->bboard & ~moving->presence) {
          move_t *move = buffer + (count++);
          move->from = p->square;
          move->to = s->square;
          move->piece = 'n';
          move->captured = s->bboard & opponent_capturable
                               ? state_piece_at(opponent, move->to)
                               : '\0';
        }
        s += 1;
      }
    } break;

    case 'k': {

      uint64_t attacked =
          state_compute_attack(opponent, moving, opponent_is_white);
      const bboard_and_square_t *s = KING_MOVES[from_offset];
      const uint64_t notok_squares = moving->presence | attacked;

      for (int i = 0; i < 8 && s->bboard; i += 1) {
        if (s->bboard & ~notok_squares) {
          move_t *move = buffer + (count++);
          move->piece = 'k';
          move->from = p->square;
          move->to = s->square;
          move->captured = s->bboard & opponent_capturable
                               ? state_piece_at(opponent, move->to)
                               : '\0';
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
          move_t *move = buffer + (count++);
          move->piece = 'k';
          move->from = SQUARE(initial_row, 4);
          move->to = SQUARE(initial_row, 6);
          move->kingside_castling = true;
        }

        if (castling->queenside && !(OCCUPIED_OR_ATTACKED(3) ||
                                     OCCUPIED_OR_ATTACKED(2) || OCCUPIED(1))) {
          move_t *move = buffer + (count++);
          move->piece = 'k';
          move->from = SQUARE(initial_row, 4);
          move->to = SQUARE(initial_row, 2);
          move->queenside_castling = true;
        }

#undef OCCUPIED_OR_ATTACKED
#undef OCCUPIED
      }
    } break;

    case 'r': {
      ROOK_MOVES('r');
    } break;
    case 'b': {
      BISHOP_MOVES('b');
    } break;
    case 'q': {
      ROOK_MOVES('q');
      BISHOP_MOVES('q');
    } break;
    case 'p': {
      int dest_row = from_row + pawn_direction;
      square_t dest_square = SQUARE(dest_row, from_col);
      uint64_t dest = BBOARD(dest_square);

      /*non capture moves*/
      if (dest & ~occupied) {

        /* one step forward */
        if (dest_row != promotion_row) {
          move_t *move = buffer + (count++);
          move->piece = 'p';
          move->from = p->square;
          move->to = dest_square;
        } else {
          /* promotion */
          for (int i = 0; i < 4; i += 1) {
            move_t *move = buffer + (count++);
            move->piece = 'p';
            move->promotion = "qnrb"[i];
            move->from = p->square;
            move->to = dest_square;
          }
        }

        /* initial move : 2 steps forward */
        if (from_row == pawn_row) {
          int jump_dest_row = from_row + pawn_direction * 2;
          square_t jump_dest = SQUARE(jump_dest_row, from_col);
          dest = BBOARD(jump_dest);
          if (dest & ~occupied) {
            move_t *move = buffer + (count++);
            move->piece = 'p';
            move->pawn_jumstart = true;
            move->from = p->square;
            move->to = jump_dest;
          }
        }
      }

#define PAWN_CAPTURE(dir)                                                      \
  do {                                                                         \
    dest_square = SQUARE(from_row + pawn_direction, from_col + (dir));         \
    dest = BBOARD(dest_square);                                                \
    if (dest & (opponent_capturable | boardstate->enpassant)) {                \
      if (dest_row != promotion_row) {                                         \
        move_t *move = buffer + (count++);                                     \
        move->piece = 'p';                                                     \
        move->from = p->square;                                                \
        move->to = dest_square;                                                \
        if (move->enpassant = dest == boardstate->enpassant) {                 \
          move->captured = 'p';                                                \
        } else {                                                               \
          move->captured = state_piece_at(opponent, dest_square);              \
        }                                                                      \
      } else {                                                                 \
        char captured = state_piece_at(opponent, dest_square);                 \
        for (int i = 0; i < 4; i += 1) {                                       \
          move_t *move = buffer + (count++);                                   \
          move->piece = 'p';                                                   \
          move->from = p->square;                                              \
          move->to = dest_square;                                              \
          move->captured = captured;                                           \
          move->promotion = "qnrb"[i];                                         \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

      /* capture left */
      if (from_col > 0) {
        PAWN_CAPTURE(-1);
      }
      if (from_col < 7) {
        PAWN_CAPTURE(1);
      }
    } break;
    }
    p += 1;

  } while (p->name);

  (buffer + count)->piece = '\0';
  return count;

#undef PAWN_CAPTURE
#undef ROOK_MOVES
#undef BISHOP_MOVES
#undef RAY_MOVES
}

char boardstate_piece_at(const boardstate_t *boardstate,
                         const square_t square) {
  char p = state_piece_at(&boardstate->white, square);
  if (p != '\0') {
    return toupper(p);
  } else {
    return state_piece_at(&boardstate->black, square);
  }
}

boardstate_t *boardstate_from_fen(boardstate_t *boardstate, const char *fen) {

  char tmp[FEN_STR_SIZE];
  char *positions, *turn, *castlings, *enpassant, *halfmoves, *moves;
  char *s;
  int i = 0;
  state_t *current;

  memset(boardstate, 0, sizeof(boardstate_t));
  strncpy(tmp, fen, FEN_STR_SIZE);
  positions = tmp;
  for (s = tmp; *s != '\0'; s += 1) {
    if (*s == ' ') {
      *s = '\0';
      switch (i) {
      case 0:
        turn = s + 1;
        i = 1;
        break;
      case 1:
        castlings = s + 1;
        i = 2;
        break;
      case 2:
        enpassant = s + 1;
        i = 3;
        break;
      case 3:
        halfmoves = s + 1;
        i = 4;
        break;
      case 4:
        moves = s + 1;
        i = 5;
        break;
      }
    }
  }
  if (i != 5) {
    return NULL;
  }

  /* positions */
  int row = 7;
  int col = 0;
  for (int i = 0, len = strlen(positions); i < len; i += 1) {

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
    current = piece == c ? &boardstate->black : &boardstate->white;
    state_enplace_piece(current, piece, SQUARE(row, col));
    col += 1;
  }

  /* turn */
  boardstate->white_to_move = turn[0] == 'w';

  /* moves */
  boardstate->moves = atoi(moves);

  /* castlings */
  boardstate->white_castling.kingside = strchr(castlings, 'K') != NULL;
  boardstate->white_castling.queenside = strchr(castlings, 'Q') != NULL;
  boardstate->black_castling.kingside = strchr(castlings, 'k') != NULL;
  boardstate->black_castling.queenside = strchr(castlings, 'q') != NULL;

  /* enpassant */
  if (enpassant[0] == '-') {
    boardstate->enpassant = 0;
  } else {
    int col = enpassant[0] - 'a';
    int row = enpassant[1] - '1';
    boardstate->enpassant = BBOARD(SQUARE(row, col));
  }

  /* halfmoves */
  boardstate->halfmoves = atoi(halfmoves);

  boardstate_recompute_zobrist_hash(boardstate);

  return boardstate;
}

char *boardstate_to_fen(const boardstate_t *boardstate, char *fen) {
  char *s = fen;

  /* position */
  for (int row = 7; row >= 0; row -= 1) {
    if (row < 7) {
      *(s++) = '/';
    }
    int empty = 0;
    for (int col = 0; col < 8; col += 1) {
      char p = boardstate_piece_at(boardstate, SQUARE(row, col));
      if (p == '\0') {
        empty += 1;
      } else {
        if (empty > 0) {
          *(s++) = _012345678[empty];
          empty = 0;
        }
        *(s++) = p;
      }
    }

    if (empty > 0) {
      *(s++) = _012345678[empty];
    }
  }

  /* current player */
  *(s++) = ' ';
  *(s++) = boardstate->white_to_move ? 'w' : 'b';

  /* castlings */
  *(s++) = ' ';
  bool has_castlings = false;
  if (boardstate->white_castling.kingside) {
    *(s++) = 'K';
    has_castlings = true;
  }
  if (boardstate->white_castling.queenside) {
    *(s++) = 'Q';
    has_castlings = true;
  }
  if (boardstate->black_castling.kingside) {
    *(s++) = 'k';
    has_castlings = true;
  }
  if (boardstate->black_castling.queenside) {
    *(s++) = 'q';
    has_castlings = true;
  }
  if (!has_castlings) {
    *(s++) = '-';
  }

  /* enpassant */
  *(s++) = ' ';
  if (boardstate->enpassant) {
    square_t e = square_for_bboard(boardstate->enpassant);
    *(s++) = abcdefgh[COL(e)];
    *(s++) = _12345678[ROW(e)];
  } else {
    *(s++) = '-';
  }

  /* half moves */
  *(s++) = ' ';
  char tmp[64];
  snprintf(tmp, 63, "%d", boardstate->halfmoves);
  strncpy(s, tmp, 63);
  s = fen + strlen(fen);

  /* moves */
  *(s++) = ' ';
  snprintf(tmp, 63, "%d", boardstate->moves);
  strncpy(s, tmp, 63);
  s = fen + strlen(fen);

  /* string terminator */
  *s = '\0';

  return fen;
}

typedef struct _predicate {
  bool (*fn)(struct _predicate *p, move_t *);
  void (*destroy_fn)(struct _predicate *p);
  void *data;
} predicate_t;

bool boardstate_is_check(const boardstate_t *boardstate) {
  const state_t *moving;
  const state_t *opponent;
  bool opponent_is_white;
  if (boardstate->white_to_move) {
    moving = &boardstate->white;
    opponent = &boardstate->black;
    opponent_is_white = false;
  } else {
    moving = &boardstate->black;
    opponent = &boardstate->white;
    opponent_is_white = true;
  }
  uint64_t attacked = state_compute_attack(opponent, moving, opponent_is_white);
  return !(attacked & moving->king);
}

static INLINE bool is_legal_move(const boardstate_t *boardstate,
                                 const move_t *move) {
  boardstate_t copy;
  memcpy(&copy, boardstate, sizeof(boardstate_t));
  state_t *moving;
  state_t *opponent;
  castling_t *castling;
  bool opponent_is_white;
  if (copy.white_to_move) {
    moving = &copy.white;
    opponent = &copy.black;
    castling = &copy.white_castling;
    opponent_is_white = false;
  } else {
    moving = &copy.black;
    opponent = &copy.white;
    castling = &copy.black_castling;
    opponent_is_white = true;
  }

  state_update_result_t moveresult;
  state_update_for_move(moving, !opponent_is_white, move, &moveresult);
  if (move->captured) {
    state_update_for_capture(opponent, opponent_is_white, move);
  }
  uint64_t attacked = state_compute_attack(opponent, moving, opponent_is_white);
  return !(moving->king & attacked);
}

bool boardstate_make_move(boardstate_t *boardstate, const move_t *move) {

  state_t *moving;
  state_t *opponent;
  castling_t *castling;
  bool opponent_is_white;

  if (boardstate->white_to_move) {
    moving = &boardstate->white;
    opponent = &boardstate->black;
    castling = &boardstate->white_castling;
    opponent_is_white = false;
  } else {
    moving = &boardstate->black;
    opponent = &boardstate->white;
    castling = &boardstate->black_castling;
    opponent_is_white = true;
  }

  /* save the states if we have to revert later */
  state_t saved_white;
  state_t saved_black;
  memcpy(&saved_white, &boardstate->white, sizeof(state_t));
  memcpy(&saved_black, &boardstate->black, sizeof(state_t));

  /* update the state for the moving side */
  state_update_result_t moveresult;
  memcpy(&moveresult.castling_rights, castling, sizeof(castling_t));
  state_update_for_move(moving, !opponent_is_white, move, &moveresult);

  /* update the opponents pieces if this is a capture */
  if (move->captured) {
    state_update_for_capture(opponent, opponent_is_white, move);
  }

  /* checks which squares are attacked now */
  uint64_t attacked = state_compute_attack(opponent, moving, opponent_is_white);
  if (moving->king & attacked) {

    // Oops, the move is illegal, we gotta to revert
    memcpy(&boardstate->white, &saved_white, sizeof(state_t));
    memcpy(&boardstate->black, &saved_black, sizeof(state_t));
    return false;

  } else {

    /* Save some values to we can update the zobrist hash */
    boardstate_memento_t memento;
    memento.castling = *castling;
    memento.enpassant = boardstate->enpassant;
    memento.halfmoves = boardstate->halfmoves;

    /* update the struct */
    boardstate->enpassant = moveresult.enpassant;
    if (boardstate->white_to_move) {
      boardstate->white_castling = moveresult.castling_rights;
      boardstate->white_to_move = false;
    } else {
      boardstate->black_castling = moveresult.castling_rights;
      boardstate->white_to_move = true;
      boardstate->moves += 1;
    }

    if (move->piece == 'p' || move->captured) {
      boardstate->halfmoves = 0;
    } else {
      boardstate->halfmoves += 1;
    }

    boardstate_evolve_zobrist_hash(boardstate, &memento, move);

    return true;
  }
}

void boardstate_get_memento(const boardstate_t *boardstate,
                            boardstate_memento_t *memento) {
  const castling_t *castling = boardstate->white_to_move
                                   ? &boardstate->white_castling
                                   : &boardstate->black_castling;
  memento->castling.kingside = castling->kingside;
  memento->castling.queenside = castling->queenside;
  memento->halfmoves = boardstate->halfmoves;
  memento->enpassant = boardstate->enpassant;
  memento->z = boardstate->z;
}

void boardstate_unmake_move(boardstate_t *boardstate, const move_t *move,
                            const boardstate_memento_t *memento) {
  state_t *hasplayed;
  state_t *opponent;
  castling_t *castling;

  if (boardstate->white_to_move) {
    hasplayed = &boardstate->black;
    opponent = &boardstate->white;
    castling = &boardstate->black_castling;
    boardstate->moves -= 1;
  } else {
    hasplayed = &boardstate->white;
    opponent = &boardstate->black;
    castling = &boardstate->white_castling;
  }

  if (move->captured) {
    if (move->enpassant) {
      int row = hasplayed == &boardstate->white ? 4 : 3;
      state_enplace_piece(opponent, 'p', SQUARE(row, COL(move->to)));
    } else {
      state_enplace_piece(opponent, move->captured, move->to);
    }
  }

  if (move->promotion) {
    state_remove_piece(hasplayed, move->to);
    state_enplace_piece(hasplayed, 'p', move->from);
  } else {
    state_move_piece(hasplayed, move->piece, move->to, move->from);
  }

  if (move->kingside_castling) {
    int row = hasplayed == &boardstate->white ? 0 : 7;
    state_move_piece(hasplayed, 'r', SQUARE(row, 5), SQUARE(row, 7));
  } else if (move->queenside_castling) {
    int row = hasplayed == &boardstate->white ? 0 : 7;
    state_move_piece(hasplayed, 'r', SQUARE(row, 3), SQUARE(row, 0));
  }

  if (boardstate->white_to_move) {
    boardstate->white_to_move = false;
    boardstate->black_castling.kingside = memento->castling.kingside;
    boardstate->black_castling.queenside = memento->castling.queenside;
  } else {
    boardstate->white_to_move = true;
    boardstate->white_castling.kingside = memento->castling.kingside;
    boardstate->white_castling.queenside = memento->castling.queenside;
  }
  boardstate->halfmoves = memento->halfmoves;
  boardstate->enpassant = memento->enpassant;
  boardstate->z = memento->z;
}

/*---------------------------------------------------------------------------*/
bool p_true(predicate_t *p, move_t *move) { return true; }

void p_destroy(predicate_t *p) { free(p); }

predicate_t *make_predicate_true() {
  predicate_t *p = malloc(sizeof(predicate_t));
  p->fn = p_true;
  p->destroy_fn = p_destroy;
  p->data = NULL;
  return p;
}
/*---------------------------------------------------------------------------*/
bool p_and(struct _predicate *p, move_t *move) {
  predicate_t *p1 = ((predicate_t **)p->data)[0];
  predicate_t *p2 = ((predicate_t **)p->data)[1];
  return p1->fn(p1, move) && p2->fn(p2, move);
}

void p_destroy_and(predicate_t *p) {
  predicate_t *p1 = ((predicate_t **)p->data)[0];
  predicate_t *p2 = ((predicate_t **)p->data)[1];
  p1->destroy_fn(p1);
  p2->destroy_fn(p2);
  free(p->data);
  free(p);
}

predicate_t * and (predicate_t * p1, predicate_t *p2) {
  predicate_t *p = malloc(sizeof(predicate_t));
  p->data = malloc(sizeof(p1) + sizeof(p2));
  ((predicate_t **)p->data)[0] = p1;
  ((predicate_t **)p->data)[1] = p2;
  p->fn = p_and;
  p->destroy_fn = p_destroy_and;
  return p;
}
/*---------------------------------------------------------------------------*/
bool p_is_kingside_castling(predicate_t *p, move_t *move) {
  return move->kingside_castling;
}

predicate_t *make_predicate_is_kingside_castling() {
  predicate_t *p = malloc(sizeof(predicate_t));
  p->fn = p_is_kingside_castling;
  p->destroy_fn = p_destroy;
  p->data = NULL;
  return p;
}

/*---------------------------------------------------------------------------*/
bool p_is_queenside_castling(predicate_t *p, move_t *move) {
  return move->queenside_castling;
}

predicate_t *make_predicate_is_queenside_castling() {
  predicate_t *p = malloc(sizeof(predicate_t));
  p->fn = p_is_queenside_castling;
  p->destroy_fn = p_destroy;
  p->data = NULL;
  return p;
}

/*---------------------------------------------------------------------------*/
bool p_promotion(predicate_t *p, move_t *move) {
  char *ptr = ((char *)&(p->data));
  return move->promotion == *ptr;
}

predicate_t *make_predicate_promotion(char piece) {
  predicate_t *p = malloc(sizeof(predicate_t));
  char *ptr = ((char *)&(p->data));
  *ptr = piece;
  p->fn = p_promotion;
  p->destroy_fn = p_destroy;
  return p;
}

/*---------------------------------------------------------------------------*/
bool p_piece(predicate_t *p, move_t *move) {
  char *ptr = ((char *)&(p->data));
  return move->piece == *ptr;
}

predicate_t *make_predicate_piece(char piece) {
  predicate_t *p = malloc(sizeof(predicate_t));
  char *ptr = ((char *)&(p->data));
  *ptr = piece;
  p->fn = p_piece;
  p->destroy_fn = p_destroy;
  return p;
}

/*---------------------------------------------------------------------------*/
bool p_capture(predicate_t *p, move_t *move) { return move->captured != '\0'; }

predicate_t *make_predicate_capture() {
  predicate_t *p = malloc(sizeof(predicate_t));
  p->fn = p_capture;
  p->destroy_fn = p_destroy;
  return p;
}

/*---------------------------------------------------------------------------*/
bool p_from_row(predicate_t *p, move_t *move) {
  char *ptr = ((char *)&(p->data));
  return *ptr == '1' + ROW(move->from);
}
predicate_t *make_predicate_from_row(char c) {
  predicate_t *p = malloc(sizeof(predicate_t));
  char *ptr = ((char *)&(p->data));
  *ptr = c;
  p->fn = p_from_row;
  p->destroy_fn = p_destroy;
  return p;
}

/*---------------------------------------------------------------------------*/
bool p_to_row(predicate_t *p, move_t *move) {
  char *ptr = ((char *)&(p->data));
  return *ptr == '1' + ROW(move->to);
}
predicate_t *make_predicate_to_row(char c) {
  predicate_t *p = malloc(sizeof(predicate_t));
  char *ptr = ((char *)&(p->data));
  *ptr = c;
  p->fn = p_to_row;
  p->destroy_fn = p_destroy;
  return p;
}

/*---------------------------------------------------------------------------*/
bool p_from_col(predicate_t *p, move_t *move) {
  char *ptr = ((char *)&(p->data));
  return *ptr == 'a' + COL(move->from);
}
predicate_t *make_predicate_from_col(char c) {
  predicate_t *p = malloc(sizeof(predicate_t));
  char *ptr = ((char *)&(p->data));
  *ptr = c;
  p->fn = p_from_col;
  p->destroy_fn = p_destroy;
  return p;
}

/*---------------------------------------------------------------------------*/
bool p_to_col(predicate_t *p, move_t *move) {
  char *ptr = ((char *)&(p->data));
  return *ptr == 'a' + COL(move->to);
}
predicate_t *make_predicate_to_col(char c) {
  predicate_t *p = malloc(sizeof(predicate_t));
  char *ptr = ((char *)&(p->data));
  *ptr = c;
  p->fn = p_to_col;
  p->destroy_fn = p_destroy;
  return p;
}

/*---------------------------------------------------------------------------*/
move_t *boardstate_get_move(const boardstate_t *boardstate, const char *s,
                            move_t *move) {
  char prec = '\0';
  bool defining_from = true;
  bool defining_promotion = false;
  char san[64];
  char *psan = san;
  int sanlen = 0;
  char first;

  predicate_t *predicate = make_predicate_true();

  for (int i = 0; s[i] != '\0'; i += 1) {
    char c = s[i];
    if (c != '!' && c != '?' && c != '#' && c != '+') {
      san[sanlen++] = c;
    }
  }
  san[sanlen] = '\0';

  // castling queenside
  if (strncmp(san, "O-O", sanlen) == 0) {
    predicate = and(predicate, make_predicate_is_kingside_castling());
    goto boardstate_get_move_end;
  }

  // castling kingside
  if (strncmp(san, "O-O-O", sanlen) == 0) {
    predicate = and(predicate, make_predicate_is_queenside_castling());
    goto boardstate_get_move_end;
  }

  first = san[0];

  // the given string is in fact an UCI-style move
  if (strchr(abcdefgh, first) != NULL) {

    if (sanlen < 6) {

      char second = san[1];

      if (isdigit(second)) {

        if (sanlen < 3) {
          predicate = and(predicate, make_predicate_to_col(first));
          predicate = and(predicate, make_predicate_to_row(second));
          predicate = and(predicate, make_predicate_piece('p'));
          goto boardstate_get_move_end;
        } else {
          char third = san[2];
          char fourth = san[3];

          if (isdigit(second) && isalpha(third) && isdigit(fourth)) {

            predicate = and(predicate, make_predicate_from_col(first));
            predicate = and(predicate, make_predicate_from_row(second));
            predicate = and(predicate, make_predicate_to_col(third));
            predicate = and(predicate, make_predicate_to_row(fourth));

            /* UCI promotion */
            if (sanlen == 5) {
              char prom = tolower(san[4]);
              predicate = and(predicate, make_predicate_promotion(prom));
            }
            goto boardstate_get_move_end;
          }
        }
      }
    }

    defining_from = false;
    predicate = and(predicate, make_predicate_from_col(first));
    predicate = and(predicate, make_predicate_piece('p'));
    psan += 1;
  }

  for (char *pc = psan; *pc != '\0'; pc += 1) {

    if (strchr("RNBQK", *pc) != NULL) {
      char piece = tolower(*pc);
      if (defining_promotion) {
        predicate = and(predicate, make_predicate_promotion(piece));
      } else {
        predicate = and(predicate, make_predicate_piece(piece));
        defining_from = sanlen > 3;
      }
    }

    else if (strchr(_12345678, *pc) != NULL) {
      if (defining_from) {
        predicate = and(predicate, make_predicate_from_row(*pc));
      } else {
        predicate = and(predicate, make_predicate_to_row(*pc));
      }
      defining_from = false;
    }

    else if (strchr(abcdefgh, *pc) != NULL) {
      if (strchr(abcdefgh, prec) != NULL) {
        defining_from = false;
      }
      if (defining_from) {
        predicate = and(predicate, make_predicate_from_col(*pc));
      } else {
        predicate = and(predicate, make_predicate_to_col(*pc));
      }
    }

    else if (*pc == 'x') {
      predicate = and(predicate, make_predicate_capture());
      defining_from = false;
    }

    else if (*pc == '=') {
      defining_promotion = true;
    }

    prec = *pc;
  }

boardstate_get_move_end : {
  /* create a temporary buffer for storing the moves, and generate them */
  move_t moves[MOVES_MAX];
  move_t *selected[MOVES_MAX];
  memset(moves, 0, sizeof(moves));
  if (boardstate_generate_moves(boardstate, moves)) {

    move_t *current = moves;
    int nselected = 0;

    /* select the moves that may correspond */
    do {
      if (predicate->fn(predicate, current)) {
        selected[nselected++] = current;
      }
      current += 1;
    } while (current->piece);

    /* if only one move corresponds, return it */
    if (nselected == 1) {
      memcpy(move, selected[0], sizeof(move_t));
      predicate->destroy_fn(predicate);
      return move;
    }

    /* if several moves could correspond, return the one that is legal */
    if (nselected > 1) {
      for (int i = 0; i < nselected; i++) {
        if (is_legal_move(boardstate, selected[i])) {
          memcpy(move, selected[i], sizeof(move_t));
          predicate->destroy_fn(predicate);
          return move;
        }
      }
    }
  }
}

  /* when no move is found, return NULL*/
  predicate->destroy_fn(predicate);
  return NULL;
}

uint64_t boardstate_attacked(const boardstate_t *boardstate) {
  const state_t *moving;
  const state_t *opponent;
  bool attacker_is_white;
  if (boardstate->white_to_move) {
    moving = &boardstate->white;
    opponent = &boardstate->black;
    attacker_is_white = false;
  } else {
    moving = &boardstate->black;
    opponent = &boardstate->white;
    attacker_is_white = true;
  }
  return state_compute_attack(opponent, moving, attacker_is_white);
}

void boardstate_count_pieces(const boardstate_t *boardstate,
                             pieces_count_t *pc) {
  pc->white_knights = popcnt64(boardstate->white.knights);
  pc->white_pawns = popcnt64(boardstate->white.pawns);
  pc->white_queens =
      popcnt64(boardstate->white.rooks & boardstate->white.bishops);
  pc->white_rooks = popcnt64(boardstate->white.rooks) - pc->white_queens;
  pc->white_bishops = popcnt64(boardstate->white.knights) - pc->white_queens;

  pc->black_knights = popcnt64(boardstate->black.knights);
  pc->black_pawns = popcnt64(boardstate->black.pawns);
  pc->black_queens =
      popcnt64(boardstate->black.rooks & boardstate->black.bishops);
  pc->black_rooks = popcnt64(boardstate->black.rooks) - pc->black_queens;
  pc->black_bishops = popcnt64(boardstate->black.knights) - pc->black_queens;
}

#ifdef __cplusplus
} // extern "C"
#endif