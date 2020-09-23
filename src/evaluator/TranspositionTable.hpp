#ifndef TranspositionTable_HPP
#define TranspositionTable_HPP

#include <list>
#include <map>
#include <mutex>

#include <boost/optional.hpp>

namespace siegbert {

struct known_score_t {
  int depth;
  int score;
  int flag;
};

#define TT_EXACT 0
#define TT_LOWERBOUND 1
#define TT_UPPERBOUND 2

class TranspositionTable {

private:
  std::mutex mutex;

  std::map<uint64_t, known_score_t> *insert_current;

  std::list<std::map<uint64_t, known_score_t>> hunks;

  std::list<std::map<uint64_t, known_score_t>>::iterator iterator;

  int n_hunks;

  int hunk_size;

  ulong hits_, miss_;

  void shrink();

public:
  TranspositionTable(int n_hunks = 5, int hunk_size = 10240);

  void reset(int n_hunks = 5, int hunk_size = 1024);

  void set(uint64_t zobrist_hash, int depth, int score, int flag);

  boost::optional<known_score_t> get(uint64_t zobrist_hash);

  ulong size() const;

  ulong hits() const;

  ulong missed() const;

  ulong requested() const;
};

} // namespace siegbert

#endif