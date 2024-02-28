#include <functional>
#include <mutex>
#include <vector>
#include <thread>
#include <queue>

#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

class ThreadPool {
    std::vector<std::thread> workers{};
    std::queue<std::function<void(int)>> task_queue{};
    std::mutex queue_mtx {};
};

#endif
