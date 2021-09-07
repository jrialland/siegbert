#include "evaluator/Negamax.hpp"
#include "evaluator/TranspositionTable.hpp"

#include <climits>
#include <algorithm>

namespace siegbert {

void Negamax::set_boardState(BoardState &boardState) {
    pBs = &boardState;
}

int Negamax::negamax(int depth, int alpha, int beta) {
    using namespace std;

    int alphaOrig = alpha;
    TTableEntry ttEntry;
    Memento memento = pBs->memento();

    if(tt.find(memento.z, ttEntry) && ttEntry.depth >= depth) {
        
        switch(ttEntry.flag) {
            case EXACT : return ttEntry.value;
            case LOWERBOUND : alpha = max(alpha, ttEntry.value); break;
            case UPPERBOUND : beta = min(beta, ttEntry.value); break;
        }
        
        if(alpha >= beta) {
            return ttEntry.value;
        }

    }

    if(depth == 0) {
        int score = scorer.getScore(*pBs);
        if(pBs->white_to_move) {
            score = - score;
        }
        return score;
    }

    vector<Move> moves = pBs->generate_moves();

    // todo : sort moves
    int value = INT_MIN;
    bool has_legal_moves = false;

    for(auto &move : moves) {
        if(pBs->make_move(move)) {
            has_legal_moves = true;
            value = max(value, -negamax(depth -1, -beta, -alpha));
            pBs->unmake_move(move, memento);
            alpha = max(alpha, value);
            if(alpha >= beta) {
                break;
            }
        }
    }

    if(!has_legal_moves) {
        if(pBs->is_check()) {
            return INT_MIN;
        } else {
            return 0;
        }
    }

    ttEntry.value = value;
    if(value <= alphaOrig) {
        ttEntry.flag = UPPERBOUND;
    } else if(value >= beta) {
        ttEntry.flag = LOWERBOUND;
    } else {
        ttEntry.flag = EXACT;
    }
    ttEntry.depth = depth;
    tt.put(memento.z, ttEntry);

    return value;
}

void Negamax::reset() {
    tt.reset();
}

}