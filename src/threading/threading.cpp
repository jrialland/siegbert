#include "threading.hpp"

#include <algorithm>

namespace siegbert {

ThreadPool::ThreadPool(int n_threads_) {
    if(n_threads_ < 1) {
        n_threads_ = std::max(1u, std::thread::hardware_concurrency());
    }
    this->n_threads = n_threads_;
    if(n_threads > 1) {
        for(int i=0; i < n_threads; i++) {
            threads.push_back(std::thread([this](void) -> void {
                while(!is_done()) {
                    std::function<void(void)> fn;
                    if(pending.try_pop(fn, 2000)) {
                        fn();
                    }
                }
            }));
        }
    }
}

ThreadPool::~ThreadPool() {
    clear_pending();
    do {
        std::unique_lock<std::mutex> lock(guard);
        done = true;
    } while(0);
    for(auto &t : threads) {
        t.join();
    }
}

void ThreadPool::clear_pending() {
    pending.clear();
}

}
