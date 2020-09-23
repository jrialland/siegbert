#include "TranspositionTable.hpp"
#include "cstring"

#include <sstream>

using namespace std;

namespace siegbert {

TranspositionTable::TranspositionTable(int n_hunks_, int hunk_size_)
    : n_hunks(n_hunks_), hunk_size(hunk_size) {
  reset();
}

void TranspositionTable::reset(int n_hunks_, int hunk_size_) {
  std::lock_guard<std::mutex> lg(mutex);
  this->hits_ = 0;
  this->miss_ = 0;
  this->n_hunks = n_hunks_;
  this->hunk_size = hunk_size_;
  hunks.clear();
  for (int i = 0; i < n_hunks; i++) {
    hunks.push_back(map<uint64_t, known_score_t>());
  }
  iterator = hunks.begin();
}

void TranspositionTable::shrink() {
  hunks.pop_front();
  hunks.push_back(map<uint64_t, known_score_t>());
  iterator = hunks.end();
  iterator--;
}

void TranspositionTable::set(uint64_t zobrist_hash, int depth, int score,
                             int flag) {
  std::lock_guard<std::mutex> lg(mutex);

  auto it = hunks.rbegin();
  auto rend = hunks.rend();

  // find an existing record
  do {
    auto m_it = it->find(zobrist_hash);

    // found
    if (m_it != it->end()) {

      // our depth is better, so update the record
      if (depth > m_it->second.depth) {
        m_it->second.score = score;
      }
      return;
    }
    it++;
  } while (it != rend);

  // insert a new record

  // if the current hunk is full
  if (iterator->size() == hunk_size) {
    // switch to the next hunk
    iterator++;
    // if all the hunks are full, we have to shrink
    if (iterator == hunks.end()) {
      shrink();
    }
  }

  (*iterator)[zobrist_hash] = known_score_t{depth, score, flag};
}

boost::optional<known_score_t> TranspositionTable::get(uint64_t zobrist_hash) {
  std::lock_guard<std::mutex> lg(mutex);

  // scan all the hunks, more recent first
  auto it = hunks.rbegin();
  auto rend = hunks.rend();
  do {
    auto m_it = it->find(zobrist_hash);
    if (m_it != it->end()) {
      hits_ += 1;
      return m_it->second;
    }
    it++;
  } while (it != rend);

  miss_ += 1;
  return boost::optional<known_score_t>();
}

ulong TranspositionTable::size() const {
  int s = 0;
  for (auto &m : hunks) {
    s += m.size();
  }
  return s;
}

ulong TranspositionTable::hits() const { return hits_; }

ulong TranspositionTable::missed() const { return miss_; }

ulong TranspositionTable::requested() const { return hits_ + miss_; }

} // namespace siegbert