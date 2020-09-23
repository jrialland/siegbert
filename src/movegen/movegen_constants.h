#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _bboard_and_square {
  uint64_t bboard;
  uint8_t square;
} bboard_and_square_t;

extern const uint64_t KING_CAPTURES[64];
extern const bboard_and_square_t KING_MOVES[64][8];

extern const uint64_t KNIGHT_CAPTURES[64];
extern const bboard_and_square_t KNIGHT_MOVES[64][8];

extern const uint64_t WHITE_PAWN_CAPTURES[64];
extern const uint64_t BLACK_PAWN_CAPTURES[64];

extern const bboard_and_square_t BISHOP_RAY_NE[64][8];
extern const bboard_and_square_t BISHOP_RAY_SE[64][8];
extern const bboard_and_square_t BISHOP_RAY_NW[64][8];
extern const bboard_and_square_t BISHOP_RAY_SW[64][8];

extern const bboard_and_square_t ROOK_RAY_N[64][8];
extern const bboard_and_square_t ROOK_RAY_S[64][8];
extern const bboard_and_square_t ROOK_RAY_E[64][8];
extern const bboard_and_square_t ROOK_RAY_W[64][8];

#ifdef __cplusplus
}
#endif

#endif
