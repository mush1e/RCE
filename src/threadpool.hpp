#include <functional>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <thread>
#include <queue>

#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

class ThreadPool {
    std::vector<std::thread> workers{};
    std::queue<std::function<void()>> task_queue{};
    std::mutex queue_mtx {};
    std::condition_variable cv{};
    bool stop {};

    public:
        explicit ThreadPool(size_t);
        ~ThreadPool();

        template<typename Func>
        void enqueue(Func&& f) {
            {
                std::unique_lock<std::mutex> lock(queue_mtx);
                task_queue.emplace(std::forward<Func>(f));
            }
            cv.notify_one();
        }
};

#endif
