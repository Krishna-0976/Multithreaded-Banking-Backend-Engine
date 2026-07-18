#include "bank.hpp"
#include "queue.hpp"
#include "recovery.hpp"

#include <iostream>
#include <thread>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        if (cond) {                                                        \
            ++g_pass;                                                      \
        } else {                                                           \
            ++g_fail;                                                      \
            cerr << "FAIL: " << #cond << " (" << __FILE__ << ":"           \
                 << __LINE__ << ")\n";                                     \
        }                                                                  \
    } while (0)

void test_deposit_increases_balance() {
    BankAccount acc(1, 1000);
    CHECK(deposit(acc, 500) == true);
    CHECK(acc.balance == 1500);
}

void test_deposit_rejects_nonpositive_amount() {
    BankAccount acc(1, 1000);
    CHECK(deposit(acc, 0) == false);
    CHECK(deposit(acc, -50) == false);
    CHECK(acc.balance == 1000);
}

void test_withdraw_succeeds_with_sufficient_funds() {
    BankAccount acc(1, 1000);
    CHECK(withdraw(acc, 400) == true);
    CHECK(acc.balance == 600);
}

void test_withdraw_fails_with_insufficient_funds() {
    BankAccount acc(1, 1000);
    CHECK(withdraw(acc, 5000) == false);
    CHECK(acc.balance == 1000);
}

void test_transfer_moves_money_between_accounts() {
    BankAccount a(1, 1000);
    BankAccount b(2, 500);
    CHECK(transfer(a, b, 300) == true);
    CHECK(a.balance == 700);
    CHECK(b.balance == 800);
}

void test_transfer_fails_and_leaves_both_accounts_untouched() {
    BankAccount a(1, 100);
    BankAccount b(2, 500);
    CHECK(transfer(a, b, 10000) == false);
    CHECK(a.balance == 100);
    CHECK(b.balance == 500);
}

void test_transfer_rejects_self_transfer() {
    BankAccount a(1, 1000);
    CHECK(transfer(a, a, 100) == false);
    CHECK(a.balance == 1000);
}

void test_concurrent_transfers_preserve_total_money() {
    BankAccount a(1, 100000);
    BankAccount b(2, 100000);
    BankAccount c(3, 100000);
    BankAccount* accs[3] = {&a, &b, &c};

    const int NUM_THREADS = 8;
    const int TRANSFERS_PER_THREAD = 2000;

    auto worker = [&](int seed) {
        for (int i = 0; i < TRANSFERS_PER_THREAD; ++i) {
            int from = (seed + i) % 3;
            int to = (seed + i + 1) % 3;
            transfer(*accs[from], *accs[to], 10);
        }
    };

    vector<thread> threads;
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back(worker, t);
    }
    for (auto& th : threads) th.join();

    Cents total = a.balance + b.balance + c.balance;
    CHECK(total == 300000);
}

void test_queue_fifo_order() {
    TransactionQueue q(4);
    TransactionTask t1; t1.transaction_id = 1;
    TransactionTask t2; t2.transaction_id = 2;
    q.push(t1);
    q.push(t2);

    TransactionTask out;
    q.pop(out);
    CHECK(out.transaction_id == 1);
    q.pop(out);
    CHECK(out.transaction_id == 2);
}

void test_queue_shutdown_unblocks_waiting_pop() {
    TransactionQueue q(4);
    TransactionTask out;
    bool pop_returned = false;

    thread waiter([&]() {
        pop_returned = q.pop(out);
    });

    this_thread::sleep_for(chrono::milliseconds(50));
    q.shutdown();
    waiter.join();

    CHECK(pop_returned == false);
}

void test_ledger_format_round_trip() {
    string line = format_ledger_line(42, TransactionType::TRANSFER, 101, 102, 2550, true);
    LedgerEntry entry{};
    CHECK(parse_ledger_line(line, entry) == true);
    CHECK(entry.transaction_id == 42);
    CHECK(entry.type == TransactionType::TRANSFER);
    CHECK(entry.from_id == 101);
    CHECK(entry.to_id == 102);
    CHECK(entry.amount == 2550);
    CHECK(entry.success == true);
}

void test_parse_rejects_malformed_line() {
    LedgerEntry entry{};
    CHECK(parse_ledger_line("this is not a ledger line", entry) == false);
    CHECK(parse_ledger_line("1|BOGUS_TYPE|0|101|500|SUCCESS", entry) == false);
    CHECK(parse_ledger_line("1|DEPOSIT|0|101|500|MAYBE", entry) == false);
}

void test_replay_ledger_reconstructs_balances() {
    BankAccount a(101, 100000);
    BankAccount b(102, 100000);
    unordered_map<uint64_t, BankAccount*> by_id{{101, &a}, {102, &b}};

    vector<LedgerEntry> entries = {
        {1, TransactionType::DEPOSIT,  0,   101, 5000,  true},
        {2, TransactionType::WITHDRAW, 0,   101, 2000,  true},
        {3, TransactionType::WITHDRAW, 0,   101, 999999, false},
        {4, TransactionType::TRANSFER, 101, 102, 1000,  true},
    };

    replay_ledger(by_id, entries);

    CHECK(a.balance == 100000 + 5000 - 2000 - 1000);
    CHECK(b.balance == 100000 + 1000);
}

void test_replay_skips_unknown_account_ids_safely() {
    BankAccount a(101, 1000);
    unordered_map<uint64_t, BankAccount*> by_id{{101, &a}};

    vector<LedgerEntry> entries = {
        {1, TransactionType::TRANSFER, 999, 101, 500, true},
    };

    replay_ledger(by_id, entries);
    CHECK(a.balance == 1000 + 500);
}

int main() {
    test_deposit_increases_balance();
    test_deposit_rejects_nonpositive_amount();
    test_withdraw_succeeds_with_sufficient_funds();
    test_withdraw_fails_with_insufficient_funds();
    test_transfer_moves_money_between_accounts();
    test_transfer_fails_and_leaves_both_accounts_untouched();
    test_transfer_rejects_self_transfer();
    test_concurrent_transfers_preserve_total_money();
    test_queue_fifo_order();
    test_queue_shutdown_unblocks_waiting_pop();
    test_ledger_format_round_trip();
    test_parse_rejects_malformed_line();
    test_replay_ledger_reconstructs_balances();
    test_replay_skips_unknown_account_ids_safely();

    cout << "\n" << g_pass << " passed, " << g_fail << " failed\n";
    return g_fail == 0 ? 0 : 1;
}