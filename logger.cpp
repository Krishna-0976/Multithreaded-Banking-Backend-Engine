#include "logger.hpp"
#include <iostream>

AsyncLogger::AsyncLogger(const std::string& filename, size_t cap)
    : file_path(filename), capacity(cap), head(0), tail(0), size(0), is_shutdown(false), log_queue(cap) {
    worker_thread = std::thread(&AsyncLogger::process_logs, this);
}

void AsyncLogger::log(const std::string& message) {
    std::unique_lock<std::mutex> lock(mtx);
    while (size == capacity && !is_shutdown) {
        not_full.wait(lock);
    }
    if (is_shutdown) return;

    log_queue[tail] = message;
    tail = (tail + 1) % capacity;
    size++;
    not_empty.notify_one();
}

void AsyncLogger::process_logs() {
    std::ofstream file(file_path, std::ios::app);
    if (!file.is_open()) return;

    while (true) {
        std::string msg;
        {
            std::unique_lock<std::mutex> lock(mtx);
            while (size == 0 && !is_shutdown) {
                not_empty.wait(lock);
            }
            if (size == 0 && is_shutdown) break;

            msg = log_queue[head];
            head = (head + 1) % capacity;
            size--;
            not_full.notify_one();
        }
        file << msg << "\n";
        file.flush();
    }
    file.close();
}

void AsyncLogger::shutdown() {
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (is_shutdown) return;
        is_shutdown = true;
        not_empty.notify_all();
        not_full.notify_all();
    }
    if (worker_thread.joinable()) {
        worker_thread.join();
    }
}

AsyncLogger::~AsyncLogger() {
    shutdown();
}