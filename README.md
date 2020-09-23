
[![Build Status](https://travis-ci.org/jrialland/siegbert.svg?branch=master)](https://travis-ci.org/jrialland/siegbert)

This is my attempt at writing a serious chess engine.

For the moment, I consider it as a framework for experiencing ideas.
It "works" enough to be able to play... quite bad


![Siegbert Tarrasch](http://www.learn-and-play-online-chess.com/image-files/siegbert-tarrasch.gif)

Existing features :
-------------------

* Moves generation :
    * [bitboard-based representation](https://www.chessprogramming.org/Bitboard_Board-Definition)
    * [0x88 square encoding](https://www.chessprogramming.org/0x88)
    * [Piece-Lists](https://www.chessprogramming.org/Bitboard_Board-Definition), avoiding to scan the whole board when generating moves
    * uses static tables for generating moves, i will probably not use "magic bitboards" because I dont understand them
    * Incremental [Zobrist hashes](https://www.chessprogramming.org/Zobrist_Hashing), compatible with polyglot opening books
    * [Make](https://www.chessprogramming.org/Make_Move)/[Unmake](https://www.chessprogramming.org/Unmake_Move) using the [memento pattern](https://en.wikipedia.org/wiki/Memento_pattern)
    * perft-validated & heavily tested

* IA :
    * minimax with alpha-beta pruning w/ [transposition table](https://www.chessprogramming.org/Transposition_Table) (needs improvement)
    * (for the moment) dummy moves sorting

TODO:
-----
* 
    * improve compatibility with UCI
    * multi-threading
    * Embedded openings books
    * nn-based moves sorter
    * better logging
    * understand what goes wrong with my minimax
    * let it become less suicidal
    * performance ++ ?
    * implement a real moves sorter (using some kickass IA algorithm)

How to compile :
----------------

plain c++17. tested with g++, might also work with clang (msvc ?)

```sh
    sudo apt install libboost-regex-dev
    cmake build
    cd build && make -j8
```

How to test :
-------------

xboard :

```sh
    xboard -debugMode true -fcp build/siegbert -scp build/siegbert
```
type ctrl-t and watch it loose against itself

uci :
(incoming)