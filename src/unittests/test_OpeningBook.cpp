#include <catch.hpp>
#include <fstream>
using namespace std;
#include "game/OpeningBook.hpp"

using namespace siegbert;

OpeningBook get_book(const string &basename) {
  string filename = "misc/" + basename;
  ifstream f(filename.c_str());
  if (!f.good()) {
    filename = "../misc/" + basename;
  }
  f.close();
  return OpeningBook::for_file(filename);
}

TEST_CASE("opening book", "[OpeningBook]") {
    OpeningBook book = get_book("Most_played_2mlj_base.bin");
    BoardState b = BoardState::initial();
    auto moves = b.generate_moves();
    book.weight_moves(b, moves);
    bool ok = false;
    for(auto &m: moves) {
        if(m.weight != 0) {
            ok = true;
            break;
        }
    }
    REQUIRE(ok);
}