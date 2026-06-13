#include "bank.hpp"
#include "queue.hpp"
#include "thread_pool.hpp"
#include "logger.hpp"
#include <iostream>
#include <deque>
#include <chrono>
#include <random>
#include <atomic>
#include <cmath>

AsyncLogger bank_logger("ledger.txt");
extern std::atomic<double> global_net_change;

int main() {
    const int NUM_ACCOUNTS = 10;
    const int NUM_TRANSACTIONS = 10000;
    const int NUM_WORKERS = 4;

    std::deque<BankAccount> accounts;
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        accounts.emplace_back(static_cast<uint64_t>(100 + i), 1000.0);
    }

    double total_initial_balance = NUM_ACCOUNTS * 1000.0;
    std::cout << "Starting randomized production stress test with " << NUM_ACCOUNTS << " accounts...\n";
    std::cout << "Writing transactions asynchronously to 'ledger.txt'...\n\n";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> account_dist(0, NUM_ACCOUNTS - 1);
    std::uniform_int_distribution<> type_dist(0, 2);
    std::uniform_real_distribution<> amount_dist(5.0, 50.0);

    TransactionQueue queue(2048);
    auto start_time = std::chrono::high_resolution_clock::now();

    {
        ThreadPool pool(NUM_WORKERS, queue);

        for (int i = 0; i < NUM_TRANSACTIONS; ++i) {
            TransactionTask task;
            task.transaction_id = i;
            
            double rand_amount = std::round(amount_dist(gen) * 100.0) / 100.0;
            task.amount = rand_amount;

            int rand_type = type_dist(gen);
            int rand_acc1 = account_dist(gen);

            if (rand_type == 0) {
                task.type = TransactionType::DEPOSIT;
                task.to_account = &accounts[rand_acc1];
            } else if (rand_type == 1) {
                task.type = TransactionType::WITHDRAW;
                task.to_account = &accounts[rand_acc1];
            } else {
                task.type = TransactionType::TRANSFER;
                int rand_acc2 = account_dist(gen);
                while (rand_acc1 == rand_acc2) {
                    rand_acc2 = account_dist(gen);
                }
                task.from_account = &accounts[rand_acc1];
                task.to_account = &accounts[rand_acc2];
            }
            
            queue.push(task);
        }
        queue.shutdown();
    } 

    bank_logger.shutdown();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end_time - start_time;

    std::cout << "Engine processed and logged " << NUM_TRANSACTIONS << " random tasks in " << elapsed.count() << " ms!\n\n";

    double total_final_balance = 0;
    std::cout << "--- Final Account Balances ---\n";
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        std::cout << "Acc " << accounts[i].account_id << ": $" << accounts[i].balance << "\n";
        total_final_balance += accounts[i].balance;
    }

    double perfect_expected_total = total_initial_balance + global_net_change.load();

    std::cout << "\n--- Integrity Audit ---\n";
    std::cout << "Total Final Money: $" << total_final_balance << "\n";
    std::cout << "Expected Total:    $" << perfect_expected_total << "\n";
    std::cout << "Audit Result:      " << (std::abs(total_final_balance - perfect_expected_total) < 0.01 ? "SUCCESS (PERFECT MATCH)" : "FAILED") << "\n";
    
    return 0;
}