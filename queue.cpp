#include "queue.hpp"

TransactionQueue::TransactionQueue(size_t cap)
    : capacity(cap), head(0), tail(0), count(0), is_shutdown(false), tasks(cap) {}

bool TransactionQueue::push(const TransactionTask& task) {
    unique_lock<mutex> lock(mtx);
    while (count == capacity && !is_shutdown) {
        not_full.wait(lock);
    }
    if (is_shutdown) return false;
    tasks[tail] = task;
    tail = (tail + 1) % capacity;
    count++;
    not_empty.notify_one();
    return true;
}

bool TransactionQueue::pop(TransactionTask& task) {
    unique_lock<mutex> lock(mtx);
    while (count == 0 && !is_shutdown) {
        not_empty.wait(lock);
    }
    if (count == 0 && is_shutdown) return false;
    task = tasks[head];
    head = (head + 1) % capacity;
    count--;
    not_full.notify_one();
    return true;
}

void TransactionQueue::shutdown() {
    unique_lock<mutex> lock(mtx);
    is_shutdown = true;
    not_empty.notify_all();
    not_full.notify_all();
}