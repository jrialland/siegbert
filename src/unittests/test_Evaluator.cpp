#include "evaluator/Evaluator.hpp"
#include <catch.hpp>

#include <iostream>
using namespace std;

using namespace siegbert;

TEST_CASE("basic eval", "[Evaluator]") {
  auto b = BoardState::initial();
  Evaluator evaluator;
  auto m = evaluator.eval(b, 3);
}