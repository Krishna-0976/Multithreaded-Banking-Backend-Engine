#include "logger.hpp"

AsyncLogger::AsyncLogger(const string& filename, size_t cap)
    : file_path(filename), capacity(cap), head(0), tail(0), count(0), is_shutdown(false), log_queue(cap) {
    worker_thread = thread(&AsyncLogger::process_logs, this);
}

void AsyncLogger::log(const string& message) {
    unique_lock<mutex> lock(mtx);
    while (count == capacity && !is_shutdown) {
        not_full.wait(lock);
    }
    if (is_shutdown) return;
    log_queue[tail] = message;
    tail = (tail + 1) % capacity;
    count++;
    not_empty.notify_one();
}

void AsyncLogger::process_logs() {
    ofstream file(file_path, ios::app);
    if (!file.is_open()) return;

    while (true) {
        string msg;
        {
            unique_lock<mutex> lock(mtx);
            while (count == 0 && !is_shutdown) {
                not_empty.wait(lock);
            }
            if (count == 0 && is_shutdown) break;
            msg = log_queue[head];
            head = (head + 1) % capacity;
            count--;
            not_full.notify_one();
        }
        file << msg << "\n";
        file.flush();
    }
    file.close();
}

void AsyncLogger::shutdown() {
    {
        unique_lock<mutex> lock(mtx);
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