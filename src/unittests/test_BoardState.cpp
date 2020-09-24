#include <catch.hpp>

#include "game/BoardState.hpp"
#include "pgn/Pgn.hpp"

#include <fstream>

using namespace std;
using namespace siegbert;

TEST_CASE("king legal moves", "[BoardState][smoke_test]") {
  auto b = BoardState::from_fen("7q/7K/8/8/8/8/8/k7 w - - 0 1");
  BoardState saved(b);
  auto moves = b.generate_moves();
  int legal = 0;
  for (auto m : moves) {
    if (b.make_move(m)) {
      legal += 1;
      b = saved; // avoid using unmake_move() here
    }
  }
  REQUIRE(legal == 2);
}

TEST_CASE("king moves when in check", "[BoardState][smoke_test]") {
  auto b = BoardState::from_fen("8/8/R7/3k4/R7/8/8/1KRR4 b - - 0 1");
  auto moves = b.generate_moves();
  REQUIRE(moves.size() == 1);
  REQUIRE(b.make_move(moves[0]));
  REQUIRE(moves[0].piece == 'k');
  REQUIRE(moves[0].from == SQUARE(4, 3));
  REQUIRE(moves[0].to == SQUARE(4, 4));
}

TEST_CASE("pinned bishop", "[BoardState][smoke_test]") {
  auto b = BoardState::from_fen(
      "rn1qk1nr/ppppbppp/8/8/8/8/PPPPQPPP/RNB1KBNR b KQkq - 0 1");
  auto moves = b.generate_moves();
  for (auto &move : moves) {
    if (move.piece == 'b') {
      REQUIRE(b.make_move(move) == false);
    }
  }
}

TEST_CASE("legal moves", "[BoardState][smoke_test]") {
  auto b = BoardState::from_fen(
      "1N5Q/2p1p1bk/2p2Rb1/8/1P4np/6pP/4K1P1/6R1 b - - 0 1");
  auto moves = b.generate_moves();
  std::vector<Move> legal_moves;
  int legal = 0;
  for (auto &move : moves) {
    auto memento = b.memento();
    if (b.make_move(move)) {
      REQUIRE((move.piece == 'k' || move.piece == 'b'));
      REQUIRE(move.captured == 'q');
      REQUIRE(move.to == SQUARE(7, 7));
      legal += 1;
      b.unmake_move(move, memento);
    }
  }
  REQUIRE(legal == 2);
}

TEST_CASE("white promotion", "[BoardState][smoke_test]") {
  auto b = BoardState::from_fen("8/3P4/8/8/1k1K4/8/8/8 w - - 0 1");
  auto moves = b.generate_moves();
  REQUIRE(moves.size() == 9); // 5 king moves + 4 promotions
  int prom_moves = 0, king_moves = 0;
  for (auto &move : moves) {
    if (move.piece == 'p') {
      REQUIRE(move.from == SQUARE(6, 3));
      REQUIRE(move.to == SQUARE(7, 3));
      REQUIRE(move.promotion != '\0');
      REQUIRE(move.captured == '\0');
      prom_moves += 1;
    } else {
      REQUIRE(move.piece == 'k');
      king_moves += 1;
    }
  }
  REQUIRE(prom_moves == 4);
  REQUIRE(king_moves == 5);
}

TEST_CASE("black promotion", "[BoardState][smoke_test]") {
  auto b = BoardState::from_fen("1K2k1n1/6b1/8/8/8/8/5p2/8 b - - 0 1");
  auto moves = b.generate_moves();
  int prom_moves = 0;
  for (auto &move : moves) {
    if (move.piece == 'p') {
      REQUIRE(move.from == SQUARE(1, 5));
      REQUIRE(move.to == SQUARE(0, 5));
      REQUIRE(move.promotion != '\0');
      prom_moves += 1;
    }
  }
  REQUIRE(prom_moves == 4);
}

TEST_CASE("zobrist", "[BoardState]") {
  REQUIRE(BoardState::initial().get_zobrist_hash() == 0x463b96181691fc9c);
}

TEST_CASE("castling white queenside", "[BoardState][smoke_test]") {
  auto b = BoardState::from_fen(
      "r2nk2r/ppp2p1p/1b6/3Npb2/1P6/P4N2/2P2PPP/R3KB1R w KQkq - 0 13");
  bool has_castling = false;
  for (Move m : b.generate_moves()) {
    if (m.queenside_castling) {
      has_castling = true;
      break;
    }
  }
  REQUIRE(has_castling);
}

void replay_moves(const Pgn &game, bool verbose) {
  auto b = BoardState::initial();
  for (auto &m : game.moves) {
    if (verbose) {
      cout << "------------" << endl;
      cout << b.to_fen() << '\t' << m << endl;
      cout << b << endl;
    }
    Move move = b.get_move(m);
    if (verbose) {
      cout << m << " " << move.to_json() << endl;
    }
    REQUIRE(b.make_move(move));
  }
}

ifstream get_file(const string &basename) {
  string filename = "games/" + basename;
  ifstream f(filename.c_str());
  if (!f.good()) {
    filename = "../games/" + basename;
    f = ifstream(filename.c_str());
  }
  return f;
}

void replay(const string &basename) {
  ifstream f = get_file(basename);
  auto games = Pgn::read(f, basename);
  for (auto &game : games) {
    replay_moves(game, false);
  }
}

TEST_CASE("VachierLagrave - all", "[BoardState][smoke_test]") {
  replay("VachierLagrave.pgn");
}

TEST_CASE("Carlsen - all", "[BoardState]") { replay("Carlsen.pgn"); }

TEST_CASE("Tal - all", "[BoardState]") { replay("Tal.pgn"); }

TEST_CASE("Tarrasch - all", "[BoardState]") { replay("Tarrasch.pgn"); }

void replay_and_unmake(const Pgn &pgn) {
  auto b = BoardState::initial();
  vector<string> fens;
  fens.push_back(b.to_fen());

  for (auto &m : pgn.moves) {
    Move move = b.get_move(m);
    auto memento = b.memento();
    if (b.make_move(move)) {

      b.unmake_move(move, memento);
      std::string newfen = b.to_fen();

      REQUIRE_THAT(newfen, Catch::Equals(fens.back()));
      b.make_move(move);
      fens.push_back(b.to_fen());
    }
  }
}

TEST_CASE("replay_and_unmake", "[BoardState][make_unmake]") {
  ifstream f = get_file("Carlsen.pgn");
  auto games = Pgn::read(f, "Carlsen.pgn");
  for (auto &game : games) {
    replay_and_unmake(game);
  }
}
