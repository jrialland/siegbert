#include <catch.hpp>
#include "evaluator/TranspositionTable.hpp"

using namespace siegbert;

TEST_CASE("smoke test", "[transposition table]") {
    TranspositionTable t;

    t.put(0x463b96181691fc9c, {.depth=21, .flag=EXACT, .value=-4});

    TTableEntry entry;
    REQUIRE(t.find(0x463b96181691fc9c, entry) == true);
    REQUIRE(entry.depth == 21);
    REQUIRE(entry.value == -4);
}