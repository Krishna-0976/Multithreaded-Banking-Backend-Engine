#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <thread>

class AsyncLogger {
public:
    AsyncLogger(const std::string& filename, size_t capacity = 4096);
    ~AsyncLogger();
    void log(const std::string& message);
    void shutdown();

private:
    std::string file_path;
    std::vector<std::string> log_queue;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
    bool is_shutdown;
    
    std::mutex mtx;
    std::condition_variable not_empty;
    std::condition_variable not_full;
    std::thread worker_thread;

    void process_logs();
};

#endif