#include "thread_pool.hpp"

ThreadPool::ThreadPool(size_t no_threads) : stop(false) {
    for (auto i = 0u; i < no_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                    std::function<void(int)> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) 
                            return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task(clientSocket);
                }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) 
        worker.join();
}

void enqueue(void (*function)(int), int clientSocket) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(function);
    }

    condition.notify_one();
}