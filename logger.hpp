#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <thread>

using namespace std;

class AsyncLogger {
public:
    AsyncLogger(const string& filename, size_t capacity = 4096);
    ~AsyncLogger();

    void log(const string& message);
    void shutdown();

private:
    string file_path;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    bool is_shutdown;
    vector<string> log_queue;
    mutex mtx;
    condition_variable not_empty;
    condition_variable not_full;
    thread worker_thread;

    void process_logs();
};

#endif