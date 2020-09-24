#include <catch.hpp>

#include "pgn/Pgn.hpp"

#include <fstream>
#include <iostream>

using namespace std;
using namespace siegbert;

TEST_CASE("read pgn", "[Pgn][smoke_test]") {

  ifstream f("games/VachierLagrave.pgn");
  if (!f.good()) {
    f = ifstream("../games/VachierLagrave.pgn");
  }

  auto games = Pgn::read(f, "VachierLagrave.pgn");
  REQUIRE(games.size() == 2750);
}