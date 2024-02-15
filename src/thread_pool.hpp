#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

class ThreadPool {
    std::vector<std::thread> workers{};
    std::queue<std::function<void(int)>> tasks {};
    std::mutex queue_mutex {};
    std::condition_variable condition {};
    bool stop;

    public:
        ThreadPool(size_t);
        ~ThreadPool();        
        static ThreadPool& get_instance();
        void enqueue(void (*fptr)(int),int);
};

#endif