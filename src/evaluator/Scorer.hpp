#ifndef Scorer_HPP
#define Scorer_HPP

#include "game/BoardState.hpp"

namespace siegbert {

typedef int centipawns_t;

class Scorer {

public:
  virtual centipawns_t score(const BoardState &boardstate) = 0;
};

/**
 * From "The System: A World Champion's Approach to Chess, Gambit Publications
 * Ltd, 1999, (ISBN 1-901983-10-2)"
 */
class Berliner1999Scorer : public Scorer {
public:
  centipawns_t score(const BoardState &boardstate) override;
};

} // namespace siegbert

#endif