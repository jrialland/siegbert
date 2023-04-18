#include <algorithm>
#include <functional>
#include <iostream>
#include <regex>
#include <vector>
using namespace std;

#include "game/BoardState.hpp"

namespace siegbert {

const regex re_xboard("^([a-h][1-8])([a-h][1-8])([nbrq])?$");

std::function<bool(const Move &)>
p_and(std::function<bool(const Move &)> left,
      std::function<bool(const Move &)> right) {
  return [left, right](const Move &m) -> bool { return left(m) && right(m); };
}

std::function<bool(const Move &)> p_from_square_matches(const string &spec) {
  const square_t square = SQUARE(spec[1] - '1', spec[0] - 'a');
  return [square](const Move &move) -> bool { return move.from == square; };
}

std::function<bool(const Move &)> p_to_square_matches(const string &spec) {
  const square_t square = SQUARE(spec[1] - '1', spec[0] - 'a');
  return [square](const Move &move) -> bool { return move.to == square; };
}

std::function<bool(const Move &)> p_promotion_matches(char piece) {
  piece = tolower(piece);
  return [piece](const Move &move) -> bool { return move.promotion == piece; };
}

std::function<bool(const Move &)> p_castling_kingside() {
  return [](const Move &move) -> bool { return move.kingside_castling; };
}

std::function<bool(const Move &)> p_castling_queenside() {
  return [](const Move &move) -> bool { return move.queenside_castling; };
}

std::function<bool(const Move &)> p_piece_is(char p) {
  p = tolower(p);
  return [p](const Move &move) -> bool { return move.piece == p; };
}

std::function<bool(const Move &)> p_is_capture() {
  return [](const Move &move) -> bool { return move.captured != '\0'; };
}

std::function<bool(const Move &)> p_from_row_matches(char c) {
  int row = c - '1';
  return [row](const Move &move) -> bool { return ROW(move.from) == row; };
}

std::function<bool(const Move &)> p_from_col_matches(char c) {
  int col = c - 'a';
  return [col](const Move &move) -> bool { return COL(move.from) == col; };
}

////////////////////////////////////////////////////////////////////////////////
Move BoardState::get_move(const string &san_) const {

  string san(san_);

  // keep only what we care about
  san.erase(remove(san.begin(), san.end(), '+'), san.end());
  san.erase(remove(san.begin(), san.end(), '#'), san.end());

  std::function<bool(const Move &)> predicate;
  bool valid_predicate = false;

  // xboard-style
  cmatch m;
  if (regex_match(san.c_str(), m, re_xboard)) {
    valid_predicate = true;
    string from;
    from.assign(m[1].first, m[1].second);
    predicate = p_from_square_matches(from);

    string to;
    to.assign(m[2].first, m[2].second);
    predicate = p_and(predicate, p_to_square_matches(to));

    if (m.length() == 5) {
      char prom = *(m[3].first);
      predicate = p_and(predicate, p_promotion_matches(tolower(prom)));
    }
  }

  else if (san.compare("O-O") == 0) {
    valid_predicate = true;
    predicate = p_castling_kingside();
  }

  else if (san.compare("O-O-O") == 0) {
    valid_predicate = true;
    predicate = p_castling_queenside();
  }

  else {

    predicate = [](const Move &) -> bool { return true; };
    bool is_pawn_move = true;
    // promotion
    int index;
    if ((index = san.find('=')) != string::npos) {
      valid_predicate = true;
      predicate = p_and(predicate, p_promotion_matches(san[index + 1]));
      san = san.substr(0, san.length() - 2);
    }

    // piece spec
    if (string("RNBQK").find(san[0]) != string::npos) {
      valid_predicate = true;
      is_pawn_move = false;
      predicate = p_and(predicate, p_piece_is(san[0]));
      san = san.substr(1);
    } else {
      predicate = p_and(predicate, p_piece_is('p'));
    }

    // capture
    if (san.find('x') != string::npos) {
      valid_predicate = true;
      predicate = p_and(predicate, p_is_capture());
      san.erase(remove(san.begin(), san.end(), 'x'), san.end());
    }

    // pawn moves
    if (san.length() == 2) {
      valid_predicate = true;
      if (is_pawn_move) {
        predicate = p_and(predicate, p_piece_is('p'));
      }
      predicate = p_and(predicate, p_to_square_matches(san));
    } else {
      valid_predicate = true;
      predicate =
          p_and(predicate, p_to_square_matches(san.substr(san.length() - 2)));
      san = san.substr(0, san.length() - 2);
      if (san.length()) {
        for (char c : san) {
          if (string("12345678").find(c) != string::npos) {
            predicate = p_and(predicate, p_from_row_matches(c));
          }
          if (string("abcdefgh").find(c) != string::npos) {
            predicate = p_and(predicate, p_from_col_matches(c));
          }
        }
      }
    }
  }

  if (valid_predicate) {

    // get the moves that match the predicate
    vector<Move> candidates;
    vector<Move> moves = generate_moves();
    for (auto &move : moves) {
      if (predicate(move)) {
        candidates.push_back(move);
      }
    }

    if (candidates.size() == 1) {
      return candidates.front();
    }

    // find which move is the legal one if there are several matches
    else if (candidates.size() > 1) {

      BoardState cpy(*this);

      for (auto &move : candidates) {
        if (cpy.make_move(move)) {
          return move;
        }
      }
    }
  }

  // no move found at this point
  throw std::invalid_argument("invalid move");
}
} // namespace siegbert
