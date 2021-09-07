#ifndef TranspositionTable_HPP
#define TranspositionTable_HPP

#include <cstdint>
#include <mutex>
#include <map>
#include <list>

namespace siegbert {

typedef enum flag_t {
    EXACT,
    LOWERBOUND,
    UPPERBOUND
} flag_t;

struct TTableEntry {
    int depth;
    flag_t flag;
    int value;
};

class TranspositionTable {
private:
    
    std::mutex mutex;

    std::list<std::map<uint64_t, TTableEntry>> hunks;

    std::list<std::map<uint64_t, TTableEntry>>::iterator iterator;

    int max_hunks;

    int max_entries_per_hunk;

    void shrink();

public:

    TranspositionTable(int max_hunks=10, int max_entries_per_hunk = 10000);

    virtual ~TranspositionTable();

    void put(uint64_t z, const TTableEntry &entry);

    bool find(uint64_t z, TTableEntry &result);

    void reset(int max_hunks=10, int max_entries_per_hunk = 10000);
};

}

#endif