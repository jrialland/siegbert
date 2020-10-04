#include "OpeningBook.hpp"

#include <fstream>
#include <streambuf>
#include <cstring>
#include <arpa/inet.h>

#include <iostream>
using namespace std;

#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) (((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
# define ntohll(x) (((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

//'>QHHL' key, move, weight, learn
#define RECORD_SIZE (sizeof(uint64_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t))


namespace siegbert {

Record Seeker::read_record() {
    char tmp[RECORD_SIZE];
    char *ptr = tmp;
    read(tmp, RECORD_SIZE);
    Record record;
    record.key = *((uint64_t*)ptr);
    record.key = ntohll(record.key);
    ptr += sizeof(uint64_t);
    record.move = *((uint16_t*)ptr);
    record.move = ntohs(record.move);
    ptr += sizeof(uint16_t);
    record.weight = *((uint16_t*)ptr);
    record.weight = ntohs(record.weight);
    ptr += sizeof(uint16_t);
    record.learn = *((uint32_t*)ptr);
    record.learn = ntohl(record.learn);
    return record;
}

class membuf : public std::streambuf {
    public:
        membuf(char *begin, uint64_t len) {
            setg(begin, begin, begin + len);
        }
};

class FileSeeker : public Seeker {

    private:

        std::ifstream f;

        uint64_t len_;

    public:
    
        FileSeeker(const string& filename) {
            f.open(filename, ifstream::in | ifstream::binary);
            if( ! f.good()) {
                throw std::invalid_argument(filename);
            }
            f.seekg(0, ios_base::end);
            len_ = f.tellg();
            f.seekg(0, ios_base::beg);
        }   

        uint64_t len() const override {
            return len_;
        }

        void jump_to(uint64_t index) override {
            f.seekg(index);
        }

        void read(char *buf, size_t len) override {
            f.read(buf, len);
        }
};

class ArraySeeker : public Seeker {

    private:

        char* ptr;

        char *current;

        uint64_t len_;

    public:

    ArraySeeker(char *ptr_, uint64_t l) : 
        ptr(ptr_),
        current(ptr_),
        len_(l) {
    }

    uint64_t len() const override {
        return len_;
    }

    void jump_to(uint64_t index) override {
        current = ptr + index;
    }

    void read(char *buf, size_t len) override {
        memcpy(buf, current, len);
    }

};

OpeningBook OpeningBook::for_file(const std::string& filename) {
    OpeningBook book;
    book.seeker = new FileSeeker(filename);
    return book;
}

OpeningBook OpeningBook::for_array(char *ptr, uint64_t len) {
    OpeningBook book;
    book.seeker = new ArraySeeker(ptr, len);
    return book;
}

OpeningBook::~OpeningBook() {
    delete seeker;
}

inline void callcb(Seeker* seeker, int foundindex, int maxindex, std::function<void(const Record&)> &cb) {
    seeker->jump_to(foundindex*RECORD_SIZE);
    Record r = seeker->read_record();
    uint64_t key = r.key;
    cb(r);    
    if(foundindex > 0) {
        int current = foundindex - 1;
        while(current >= 0) {
            seeker->jump_to(current*RECORD_SIZE);
            r = seeker->read_record();
            if(r.key == key) {
                cb(r);
                current -= 1;
            } else {
                break;
            }
        }
    }
    if(foundindex < maxindex) {
        int current = foundindex + 1;
        while(current <= maxindex) {
            seeker->jump_to(current*RECORD_SIZE);
            r = seeker->read_record();
            if(r.key == key) {
                cb(r);
                current += 1;
            } else {
                break;
            }
        }
    }
}

void OpeningBook::find(
    uint64_t searched,
    int minindex,
    int maxindex,
    std::function<void(const Record&)> cb
) {
   if(minindex >= maxindex) {
        seeker->jump_to(std::min(minindex, maxindex)*RECORD_SIZE);
        Record record = seeker->read_record();
        if(record.key == searched) {
            callcb(seeker, minindex, minindex, cb);
        }
        return;
    }
    int split = minindex + (maxindex - minindex) / 2;
    seeker->jump_to(split*RECORD_SIZE);
    Record record = seeker->read_record();
    if(record.key == searched) {
        callcb(seeker, split, maxindex, cb);
    } else if(record.key > searched) {
        find(searched, minindex, split-1, cb);
    } else {
        find(searched, split+1, maxindex, cb);
    }
}

void OpeningBook::weight_moves(const BoardState& b, std::vector<Move> &moves) {
    bool white = b.is_white_to_move();
    uint64_t z = b.get_zobrist_hash();
    Record r;
    find(z, 0, (seeker->len()/RECORD_SIZE)-1, [white, &moves](const Record& record) {
        for(auto &move : moves) {
            if(move.to_polyglot(white) == record.move) {
                move.weight = htons(record.weight);
            }
        }
    });
}

}