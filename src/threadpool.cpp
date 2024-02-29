#include "threadpool.hpp"

ThreadPool::ThreadPool(size_t num_threads) {
    for(auto i = 0u; i < num_threads; i++) {
        this->workers.emplace_back( [this] {
            for(;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mtx);
                    this->cv.wait(lock, [this] { return this->stop || !this->task_queue.empty(); });
                    if (this->stop && this->task_queue.empty()) return;
                    task = std::move(this->task_queue.front());
                    this->task_queue.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mtx);
        stop = true;
    }
    cv.notify_all();
    for (std::thread& thread : workers) {
        thread.join();
    }
}
