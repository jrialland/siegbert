#include <catch.hpp>

#include "game/BoardState.hpp"
#include "logging/Logging.hpp"
#include "pgn/Pgn.hpp"

#include <fstream>

using namespace std;
using namespace siegbert;

TEST_CASE("xboard", "[BoardState][smoke_test]") {
  auto b = BoardState::initial();
  Move m = b.get_move("e2e4");
  REQUIRE(m.from == SQUARE(1, 4));
  REQUIRE(m.to == SQUARE(3, 4));
  REQUIRE(m.piece == 'p');
  REQUIRE(m.captured == '\0');
  REQUIRE(m.enpassant == false);
  REQUIRE(m.pawn_jumstart == true);
  REQUIRE(m.kingside_castling == false);
  REQUIRE(m.queenside_castling == false);
  REQUIRE(m.promotion == '\0');
}

TEST_CASE("xboard promotion", "[BoardState][smoke_test]") {
  auto b = BoardState::from_fen(
      "2r3k1/8/1p5B/pP1P2P1/PbN4p/7P/1R1PR1p1/8 b - - 0 1");
  bool haspawn = false;
  for (auto &move : b.generate_moves()) {
    if (move.from == SQUARE(1, 6)) {
      REQUIRE(move.piece == 'p');
      REQUIRE(move.to == SQUARE(0, 6));
      REQUIRE(move.promotion != '\0');
      haspawn = true;
    }
  }
  REQUIRE(haspawn);
  Move m = b.get_move("g2g1n");
}

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

// http://hgm.nubati.net/book_format.html
TEST_CASE("zobrist", "[BoardState]") {
  vector<string> fens = {
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
      "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
      "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2",
      "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2",
      "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
      "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3",
      "rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4",
      "rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3",
      "rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4"};

  vector<uint64_t> hashes = {
      0x463b96181691fc9c, 0x823c9b50fd114196, 0x0756b94461c50fb0,
      0x662fafb965db29d4, 0x22a48b5a8e47ff78, 0x652a607ca3f242c1,
      0x00fdd303c946bdd9, 0x3c8123ea7b067637, 0x5c3f9b829b279560};

  auto itFens = fens.begin();
  auto itHashes = hashes.begin();
  while (itFens != fens.end()) {
    REQUIRE(BoardState::from_fen(*itFens).get_zobrist_hash() == *itHashes);
    itFens++;
    itHashes++;
  }

  vector<string> moves = {"e2e4", "d7d5", "e4e5", "f7f5", "e1e2", "e8f7"};

  int i = 1;
  BoardState b = BoardState::initial();
  auto it = hashes.begin();
  for (auto &m : moves) {
    b.make_move(b.get_move(m));
    it++;
    REQUIRE(b.get_zobrist_hash() == *it);
  }
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

void replay_moves(const Pgn &game) {
  auto b = BoardState::initial();
  for (auto &m : game.moves) {
    // LOG_TRACE(b.to_fen(), "\t", m);
    Move move = b.get_move(m);
    // LOG_TRACE(m, " ", move.to_json());
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
  auto start = chrono::steady_clock::now();
  ifstream f = get_file(basename);
  auto games = Pgn::read(f, basename);
  int moves = 0;
  for (auto &game : games) {
    moves += game.moves.size();
    replay_moves(game);
  }
  auto end = chrono::steady_clock::now();
  auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
  LOG_INFO(basename, ":", moves, " moves,", moves * 1000.0 / ms,
           " moves per sec");
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

      REQUIRE(b.get_zobrist_hash() == memento.z);

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
  int moves = 0;
  auto start = chrono::steady_clock::now();

  for (auto &game : games) {
    moves += game.moves.size();
    replay_and_unmake(game);
  }
  auto end = chrono::steady_clock::now();
  auto ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
  LOG_INFO("make/unmake (Carlsen.pgn) :", moves, "moves,", moves * 1000.0 / ms,
           "moves per sec");
}
