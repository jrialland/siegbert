#include <catch.hpp>

#include "game/BoardState.hpp"

using namespace siegbert;

unsigned long perft(BoardState &boardstate, int depth) {
  if (depth == 0) {
    return 1L;
  }
  unsigned long nodes = 0;
  auto memento = boardstate.memento();
  auto moves = boardstate.generate_moves();
  for (auto move : moves) {
    if (boardstate.make_move(move)) {
      nodes += perft(boardstate, depth - 1);
      boardstate.unmake_move(move, memento);
    }
  }
  return nodes;
}

TEST_CASE("perft 0", "[perft][smoke_test]") {
  auto b = BoardState::initial();
  REQUIRE(perft(b, 0) == 1);
}

TEST_CASE("perft 1", "[perft][smoke_test]") {
  auto b = BoardState::initial();
  REQUIRE(perft(b, 1) == 20);
}

TEST_CASE("perft 2", "[perft][smoke_test]") {
  auto b = BoardState::initial();
  REQUIRE(perft(b, 2) == 400);
}

TEST_CASE("perft 3", "[perft]") {
  auto b = BoardState::initial();
  REQUIRE(perft(b, 3) == 8902);
}

TEST_CASE("perft 4", "[perft]") {
  auto b = BoardState::initial();
  REQUIRE(perft(b, 4) == 197281);
}

/*
TEST_CASE("perft 5", "[perft]") {
    auto b = BoardState::initial();
    REQUIRE(perft(b, 5) == 4865609);
}


TEST_CASE("perft 6", "[perft]") {
    auto b = BoardState::initial();
    REQUIRE(perft(b, 6) == 119060324);
}


TEST_CASE("perft 7", "[perft]") {
    auto b = BoardState::initial();
    REQUIRE(perft(b, 7) == 3195901860);
}


TEST_CASE("perft 8", "[perft]") {
    auto b = BoardState::initial();
    REQUIRE(perft(b, 8) == 84998978956);
}
*/