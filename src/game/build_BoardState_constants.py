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


class Declaration:
    def __init__(self, t, name, definition):
        self.t = t
        self.name = name
        self.definition = definition

    def as_decl(self):
        i = self.t.index('[')
        if i > 0:
            return f'extern const {self.t[:i]} {self.name}{self.t[i:]};'
        else:
            return f'extern const {self.t} {self.name};'

    def as_def(self):
        return self.as_decl()[:-1].replace('extern', '') + ' = ' + self.definition + ';'

def define_struct(row, col) -> str:
    return '{'+bboard_hex(row, col) +',' + sq88(row, col)+'}'

def make_array(name, movegen) -> Declaration:
		s = '{\n'
		for row in range(8):
			for col in range(8):
				bboard = 0
				for r, c in movegen(row, col): bboard = bboard | (1 << (r*8+c))
				s += f'  /*{coord(row, col)}*/ {hex(bboard)}UL,' + '\n'
		s += '}'
		return Declaration('uint64_t[64]', name, s)

def make_list_of_moves(name, movegen) -> Declaration :
    s = '{\n'
    for row in range(8):
        for col in range(8):
            items = list(map(lambda x : define_struct(*x), movegen(row, col)))
            while len(items) < 8:
                items.append('{0,0x88}')
            array = '{' + ','.join(items) + '},\n'
            s+= f'  /*{coord(row, col)}*/ ' + array
    s += '}'
    return Declaration('bboard_and_square_t[64][8]', name, s)

declarations = [
    make_array('KING_CAPTURES', king_moves),
    make_list_of_moves('KING_MOVES', king_moves),
    make_array('KNIGHT_CAPTURES', knight_moves),
    make_list_of_moves('KNIGHT_MOVES', knight_moves),
    make_array('WHITE_PAWN_CAPTURES', white_pawn_captures),
    make_array('BLACK_PAWN_CAPTURES', black_pawn_captures),
    make_list_of_moves('BISHOP_RAY_NE', bishop_ray_ne),
    make_list_of_moves('BISHOP_RAY_SE', bishop_ray_se),
    make_list_of_moves('BISHOP_RAY_NW', bishop_ray_nw),
    make_list_of_moves('BISHOP_RAY_SW', bishop_ray_sw),
    make_list_of_moves('ROOK_RAY_N', rook_ray_n),
    make_list_of_moves('ROOK_RAY_S', rook_ray_s),
    make_list_of_moves('ROOK_RAY_E', rook_ray_e),
    make_list_of_moves('ROOK_RAY_W', rook_ray_w),
]

h_pre = """
#ifndef BoardState_constants_HPP
#define BoardState_constants_HPP

#include <stdint.h>

extern "C" {

struct bboard_and_square_t {
    uint64_t bboard;
    uint8_t square;
};

"""

h_post = """
}

#endif
"""

cpp_pre = """
#include "BoardState_constants.hpp"
extern "C" {
"""

cpp_post = """
}
"""

if __name__ == '__main__':
    filename = 'BoardState_constants'

    with open(filename + '.hpp', 'w') as out:
        out.write(h_pre) 
        for d in declarations:
            out.write(d.as_decl())
            out.write('\n')
        out.write(h_post) 
    
    with open(filename + '.cpp', 'w') as out:
        out.write(cpp_pre) 
        for d in declarations:
            out.write(d.as_def())
            out.write('\n')
        out.write(cpp_post) 