#include "bank.hpp"
#include "queue.hpp"
#include "thread_pool.hpp"
#include "logger.hpp"
#include "recovery.hpp"
#include <iostream>
#include <deque>
#include <chrono>
#include <random>
#include <atomic>
#include <unordered_map>

const string LEDGER_PATH = "ledger.txt";

AsyncLogger bank_logger(LEDGER_PATH);
extern atomic<int64_t> global_net_change;

int main() {
    const int NUM_ACCOUNTS = 10;
    const int NUM_TRANSACTIONS = 10000;
    const int NUM_WORKERS = 4;
    const Cents INITIAL_BALANCE = dollars_to_cents(1000.0);

    deque<BankAccount> accounts;
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        accounts.emplace_back(static_cast<uint64_t>(100 + i), INITIAL_BALANCE);
    }
    Cents total_initial_balance = static_cast<Cents>(NUM_ACCOUNTS) * INITIAL_BALANCE;

    cout << "Starting randomized production stress test with " << NUM_ACCOUNTS << " accounts...\n";
    cout << "Writing transactions asynchronously to '" << LEDGER_PATH << "'...\n\n";

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> account_dist(0, NUM_ACCOUNTS - 1);
    uniform_int_distribution<> type_dist(0, 2);
    uniform_real_distribution<> amount_dist(5.0, 50.0);

    TransactionQueue queue(2048);
    auto start_time = chrono::high_resolution_clock::now();

    {
        ThreadPool pool(NUM_WORKERS, queue);
        for (int i = 0; i < NUM_TRANSACTIONS; ++i) {
            TransactionTask task;
            task.transaction_id = i;
            task.amount = dollars_to_cents(amount_dist(gen));

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
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> elapsed = end_time - start_time;

    cout << "Engine processed and logged " << NUM_TRANSACTIONS
         << " random tasks in " << elapsed.count() << " ms!\n\n";

    Cents total_final_balance = 0;
    cout << "--- Final Account Balances (live, in-memory) ---\n";
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        cout << "Acc " << accounts[i].account_id << ": $"
             << cents_to_dollars(accounts[i].balance) << "\n";
        total_final_balance += accounts[i].balance;
    }

    Cents expected_total = total_initial_balance + global_net_change.load();
    cout << "\n--- Integrity Audit ---\n";
    cout << "Total Final Money: $" << cents_to_dollars(total_final_balance) << "\n";
    cout << "Expected Total:    $" << cents_to_dollars(expected_total) << "\n";
    cout << "Audit Result: "
         << (total_final_balance == expected_total ? "SUCCESS (EXACT MATCH)" : "FAILED")
         << "\n";

    cout << "\n--- Simulating Crash Recovery from ledger.txt ---\n";

    deque<BankAccount> recovered_accounts;
    unordered_map<uint64_t, BankAccount*> accounts_by_id;
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        recovered_accounts.emplace_back(static_cast<uint64_t>(100 + i), INITIAL_BALANCE);
    }
    for (auto& acc : recovered_accounts) {
        accounts_by_id[acc.account_id] = &acc;
    }

    vector<LedgerEntry> entries = load_ledger(LEDGER_PATH);
    cout << "Loaded " << entries.size() << " ledger entries.\n";
    replay_ledger(accounts_by_id, entries);

    Cents recovered_total = 0;
    bool all_match = true;
    for (int i = 0; i < NUM_ACCOUNTS; ++i) {
        Cents recovered_balance = recovered_accounts[i].balance;
        Cents live_balance = accounts[i].balance;
        recovered_total += recovered_balance;
        if (recovered_balance != live_balance) {
            all_match = false;
            cout << "MISMATCH on Acc " << recovered_accounts[i].account_id
                 << ": recovered=$" << cents_to_dollars(recovered_balance)
                 << " live=$" << cents_to_dollars(live_balance) << "\n";
        }
    }

    cout << "Recovered Total: $" << cents_to_dollars(recovered_total) << "\n";
    cout << "Recovery Result: "
         << (all_match ? "SUCCESS (recovered state == live state)" : "FAILED")
         << "\n";

    return 0;
}