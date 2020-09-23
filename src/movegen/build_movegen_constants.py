import sys

def is_valid(row, col):
		return 0<= row < 8 and 0<= col < 8

def coord(row, col):
		return 'abcdefgh'[col] + str(row+1)

def sq88(row, col):
    return hex(16 * row + col)

def bboard_hex(row, col):
    bboard = 1 << (row*8+col)
    return hex(bboard)+'UL'

def knight_moves(row, col):
		for drow, dcol in [(1,2),(1,-2),(-1,2),(-1,-2),(2,1),(2,-1),(-2,1),(-2,-1)]:
			i,j = row+drow, col+dcol
			if is_valid(i,j):
				yield(i,j)

def white_pawn_captures(row, col):
    if is_valid(row+1, col-1) : yield row+1, col-1
    if is_valid(row+1, col+1) : yield row+1, col+1

def black_pawn_captures(row, col):
    if is_valid(row-1, col-1) : yield row-1, col-1
    if is_valid(row-1, col+1) : yield row-1, col+1
    
def king_moves(row, col):
    for drow in [-1,0,1]:
        for dcol in [-1,0,1]:
            if (drow, dcol) != (0,0) and is_valid(row+drow, col+dcol):
                yield (row+drow, col+dcol)

def bishop_ray(row, col, drow, dcol):
    row = row+drow
    col = col+dcol
    while is_valid(row, col):
        yield row, col
        row = row+drow
        col = col+dcol

def bishop_ray_ne(row, col) : yield from bishop_ray(row, col,  1 ,  1)
def bishop_ray_se(row, col) : yield from bishop_ray(row, col, -1 ,  1)
def bishop_ray_nw(row, col) : yield from bishop_ray(row, col,  1 , -1)
def bishop_ray_sw(row, col) : yield from bishop_ray(row, col, -1 , -1)

def rook_ray_n(row, col) : yield from bishop_ray(row, col,  1,  0)
def rook_ray_s(row, col) : yield from bishop_ray(row, col, -1,  0)
def rook_ray_e(row, col) : yield from bishop_ray(row, col,  0,  1)
def rook_ray_w(row, col) : yield from bishop_ray(row, col,  0, -1)

def make_array(name, movegen):
		s = f'const uint64_t {name}[64] = ' + '{\n'
		for row in range(8):
			for col in range(8):
				bboard = 0
				for r, c in movegen(row, col): bboard = bboard | (1 << (r*8+c))
				s += f'  /*{coord(row, col)}*/ {hex(bboard)}UL,' + '\n'
		s += '};'
		return s

def make_struct(row, col):
    return '{'+bboard_hex(row, col) +',' + sq88(row, col)+'}'

def make_list_of_moves(name, movegen):
    s = f'const bboard_and_square_t {name}[64][8] = ' + '{\n'
    for row in range(8):
        for col in range(8):
            items = list(map(lambda x : make_struct(*x), movegen(row, col)))
            while len(items) < 8:
                items.append('{0,0x88}')
            array = '{' + ','.join(items) + '},\n'
            s+= f'  /*{coord(row, col)}*/ ' + array
    s += '};'
    return s

if __name__ == '__main__':
    if '--header' in sys.argv:
        print("""#ifndef MOVEGEN_CONSTANTS_H
#define MOVEGEN_CONSTANTS_H

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
""")
    else:
        print('#include "movegen_constants.h"')
        print("""
#ifdef __cplusplus
extern "C" {
#endif
""")
        print(make_array('KING_CAPTURES', king_moves))
        print(make_list_of_moves('KING_MOVES', king_moves))
        print(make_array('KNIGHT_CAPTURES', knight_moves))
        print(make_list_of_moves('KNIGHT_MOVES', knight_moves))
        print(make_array('WHITE_PAWN_CAPTURES', white_pawn_captures))
        print(make_array('BLACK_PAWN_CAPTURES', black_pawn_captures))
        print(make_list_of_moves('BISHOP_RAY_NE', bishop_ray_ne))
        print(make_list_of_moves('BISHOP_RAY_SE', bishop_ray_se))
        print(make_list_of_moves('BISHOP_RAY_NW', bishop_ray_nw))
        print(make_list_of_moves('BISHOP_RAY_SW', bishop_ray_sw))
        print(make_list_of_moves('ROOK_RAY_N', rook_ray_n))
        print(make_list_of_moves('ROOK_RAY_S', rook_ray_s))
        print(make_list_of_moves('ROOK_RAY_E', rook_ray_e))
        print(make_list_of_moves('ROOK_RAY_W', rook_ray_w))
        print("""
#ifdef __cplusplus
}
#endif
""")
