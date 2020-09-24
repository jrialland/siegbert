#include "evaluator/Evaluator.hpp"
#include <catch.hpp>

#include <iostream>
using namespace std;

using namespace siegbert;

TEST_CASE("basic eval", "[Evaluator]") {
  auto b = BoardState::initial();
  Evaluator ev;
  std::string move = ev.eval(b, 3);
  // cout << move << endl;
}