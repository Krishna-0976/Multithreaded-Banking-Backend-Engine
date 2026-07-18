#include "thread_pool.hpp"
#include "logger.hpp"
#include "recovery.hpp"
#include <atomic>

extern AsyncLogger bank_logger;
atomic<int64_t> global_net_change{0};

ThreadPool::ThreadPool(size_t num_threads, TransactionQueue& q) : queue(q) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back(&ThreadPool::worker_routine, this);
    }
}

void ThreadPool::worker_routine() {
    TransactionTask task;
    while (queue.pop(task)) {
        bool success = false;
        uint64_t from_id = 0;
        uint64_t to_id = 0;

        switch (task.type) {
            case TransactionType::DEPOSIT:
                success = deposit(*task.to_account, task.amount);
                to_id = task.to_account->account_id;
                if (success) global_net_change.fetch_add(task.amount);
                break;

            case TransactionType::WITHDRAW:
                success = withdraw(*task.to_account, task.amount);
                to_id = task.to_account->account_id;
                if (success) global_net_change.fetch_sub(task.amount);
                break;

            case TransactionType::TRANSFER:
                success = transfer(*task.from_account, *task.to_account, task.amount);
                from_id = task.from_account->account_id;
                to_id = task.to_account->account_id;
                break;
        }

        bank_logger.log(format_ledger_line(task.transaction_id, task.type,
                                            from_id, to_id, task.amount, success));
    }
}

ThreadPool::~ThreadPool() {
    queue.shutdown();
    for (thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}