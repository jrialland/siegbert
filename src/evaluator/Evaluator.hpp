#pragma once
#ifndef Evaluator_HPP
#define Evaluator_HPP

#include <climits>
#include <string>

#include "game/BoardState.hpp"
#include "evaluator/Scorer.hpp"
#include "evaluator/Negamax.hpp"

namespace siegbert {
class Evaluator {
private:

    Negamax negamax;

public:

    Evaluator();

    std::string eval(BoardState& boardstate, int depth=10);

    void reset();

};
}

#endif
