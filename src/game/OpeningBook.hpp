#ifndef OpeningBook_HPP
#define OpeningBook_HPP

#include <string>
#include <functional>
#include "game/BoardState.hpp"

namespace siegbert {

struct Record {
    uint64_t key;
    uint16_t move;
    uint16_t weight;
    uint32_t learn;
};

class Seeker {
    
    private:

    public:
        virtual uint64_t len() const=0;
        virtual void jump_to(uint64_t index)=0;
        virtual void read(char *buf, size_t len)=0;
        Record read_record();
};

class OpeningBook {

    private:

        Seeker  *seeker;

        void find(
            uint64_t searched,
            int minindex,
            int maxindex, std::function<void(const Record&)> cb);

    public:
        ~OpeningBook();

    static OpeningBook for_file(const std::string& filename);

    static OpeningBook for_array(char *ptr, uint64_t len);

    void weight_moves(const BoardState& boardstate, std::vector<Move> &moves);
};
}

#endif