#include "TranspositionTable.hpp"

namespace siegbert {

TranspositionTable::TranspositionTable(int _max_hunks,
                                       int _max_entries_per_hunk)
    : max_hunks(_max_hunks), max_entries_per_hunk(_max_entries_per_hunk) {
  reset(max_hunks, max_entries_per_hunk);
}

TranspositionTable::~TranspositionTable() {}

void TranspositionTable::put(uint64_t z, const TTableEntry &entry) {
  std::lock_guard<std::mutex> lg(mutex);

  auto it = hunks.rbegin();
  auto rend = hunks.rend();

  do {
    auto m_it = it->find(z);
    if (m_it != it->end()) {
      m_it->second = entry;
      return;
    }
    ++it;
  } while (it != rend);

  if (iterator->size() == max_entries_per_hunk) {
    iterator++;
    if (iterator == hunks.end()) {
      shrink();
    }
  }

  (*iterator)[z] = entry;
}

void TranspositionTable::shrink() {
  using namespace std;
  hunks.pop_front();
  hunks.push_back(map<uint64_t, TTableEntry>());
  iterator = hunks.end();
  iterator--;
}

bool TranspositionTable::find(uint64_t z, TTableEntry &result) {
  std::lock_guard<std::mutex> lg(mutex);

  // scan all the hunks, more recent first
  auto it = hunks.rbegin();
  auto rend = hunks.rend();
  do {
    auto m_it = it->find(z);
    if (m_it != it->end()) {
      result = m_it->second;
      return true;
    }
    it++;
  } while (it != rend);
  return false;
}

void TranspositionTable::reset(int _max_hunks, int _max_entries_per_hunk) {
  std::lock_guard<std::mutex> lg(mutex);
  this->max_hunks = _max_hunks;
  this->max_entries_per_hunk = _max_entries_per_hunk;
  hunks.clear();
  for (int i = 0; i < max_hunks; ++i) {
    hunks.push_back(std::map<uint64_t, TTableEntry>());
  }
  iterator = hunks.begin();
}

} // namespace siegbert