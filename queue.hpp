#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "bank.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>

class TransactionQueue {
public:
    TransactionQueue(size_t capacity = 1024);
    bool push(const TransactionTask& task);
    bool pop(TransactionTask& task);
    void shutdown();

private:
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    bool is_shutdown;
    vector<TransactionTask> tasks;
    mutex mtx;
    condition_variable not_empty;
    condition_variable not_full;
};

#endif