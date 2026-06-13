#include "thread_pool.hpp"
#include "logger.hpp"
#include <string>
#include <atomic>

extern AsyncLogger bank_logger;

std::atomic<double> global_net_change{0.0};

void add_to_atomic_double(std::atomic<double>& atom, double val) {
    double current = atom.load();
    while (!atom.compare_exchange_weak(current, current + val));
}

ThreadPool::ThreadPool(size_t num_threads, TransactionQueue& q) : queue(q) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back(&ThreadPool::worker_routine, this);
    }
}

void ThreadPool::worker_routine() {
    TransactionTask task;
    while (queue.pop(task)) {
        std::string log_msg;
        bool success = false;

        switch (task.type) {
            case TransactionType::DEPOSIT:
                success = deposit(*task.to_account, task.amount);
                if (success) {
                    add_to_atomic_double(global_net_change, task.amount);
                    log_msg = "DEPOSIT | Tx: " + std::to_string(task.transaction_id) + " | Acc: " + std::to_string(task.to_account->account_id) + " | Amt: $" + std::to_string(task.amount) + " | SUCCESS";
                } else {
                    log_msg = "DEPOSIT | Tx: " + std::to_string(task.transaction_id) + " | FAILED";
                }
                break;
            case TransactionType::WITHDRAW:
                success = withdraw(*task.to_account, task.amount);
                if (success) {
                    add_to_atomic_double(global_net_change, -task.amount);
                    log_msg = "WITHDRAW | Tx: " + std::to_string(task.transaction_id) + " | Acc: " + std::to_string(task.to_account->account_id) + " | Amt: $" + std::to_string(task.amount) + " | SUCCESS";
                } else {
                    log_msg = "WITHDRAW | Tx: " + std::to_string(task.transaction_id) + " | Acc: " + std::to_string(task.to_account->account_id) + " | INSUFFICIENT FUNDS";
                }
                break;
            case TransactionType::TRANSFER:
                success = transfer(*task.from_account, *task.to_account, task.amount);
                if (success) {
                    log_msg = "TRANSFER | Tx: " + std::to_string(task.transaction_id) + " | From: " + std::to_string(task.from_account->account_id) + " -> To: " + std::to_string(task.to_account->account_id) + " | Amt: $" + std::to_string(task.amount) + " | SUCCESS";
                } else {
                    log_msg = "TRANSFER | Tx: " + std::to_string(task.transaction_id) + " | From: " + std::to_string(task.from_account->account_id) + " | INSUFFICIENT FUNDS";
                }
                break;
        }
        bank_logger.log(log_msg);
    }
}

ThreadPool::~ThreadPool() {
    queue.shutdown();
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}