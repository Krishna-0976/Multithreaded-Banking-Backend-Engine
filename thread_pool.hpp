#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "queue.hpp"
#include <vector>
#include <thread>

class ThreadPool {
public:
    ThreadPool(size_t num_threads, TransactionQueue& q);
    ~ThreadPool();

private:
    std::vector<std::thread> workers;
    TransactionQueue& queue;
    void worker_routine();
};

#endif