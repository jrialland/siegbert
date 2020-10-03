#ifndef ThreadPool_HPP
#define ThreadPool_HPP
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <functional>

namespace siegbert {

template<typename T>
class blocking_queue {

    private:

        std::queue<T> queue;
        mutable std::mutex guard;
        std::condition_variable signal;

    public:

        void push(const T& data) {
            do {
                std::lock_guard<std::mutex> lock(guard);
                queue.push(data);
            } while(0);
            signal.notify_one();
        }

        T pop() {
            std::unique_lock<std::mutex> lock(guard);
            while (queue.empty()) {
                signal.wait(lock);
            }
            T value = queue.front();
            queue.pop();
            return value;
        }

        bool try_pop(T& t, int ms) {
            std::unique_lock<std::mutex> lock(guard);
            while (queue.empty()) {
                if(std::cv_status::timeout == signal.wait_for(lock, std::chrono::milliseconds(ms))) {
                    return false;
                }
            }
            t = queue.front();
            queue.pop();
            return true;
        }

        void clear() {
            std::unique_lock<std::mutex> lock(guard);
            while(!queue.empty()) {
                queue.pop();
            }
        }
};

class ThreadPool;

template<typename T>
class Future {

    private:
        
        std::mutex guard;

        std::condition_variable signal;

        T result;
        
        bool present;

        Future() : present(false) {}
        void set(const T& result) {
            do {
                std::unique_lock<std::mutex> lock(guard);
                this->result = result;
                present = true;
            } while(0);
            signal.notify_one();
        }

        friend class ThreadPool;

    public:

        T get() {
            std::unique_lock<std::mutex> lock(guard);
            while( ! present) {
                signal.wait(lock);
            }
            return result;
        }
};

class ThreadPool {
    
    private:

        std::vector<std::thread> threads;

        blocking_queue<std::function<void(void)>> pending;

        std::mutex guard;

        int n_threads;

        bool done;

        bool is_done() {
            std::unique_lock<std::mutex> lock(guard);
            return done;
        }
    public:

        ThreadPool(int n_threads=0);
        ~ThreadPool();

        template<typename T>
        Future<T>* submit(std::function<T(void)> fn) {
            Future<T> *future = new Future<T>();
            if(n_threads > 1) {
                pending.push([future, fn](void) -> void {
                    future->set(fn());
                });
            } else {
                future->set(fn());
            }
            return future;
        }

        void clear_pending();
};


}

#endif
