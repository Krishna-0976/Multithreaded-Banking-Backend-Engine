#ifndef BANK_HPP
#define BANK_HPP

#include <cstdint>
#include <mutex>

using namespace std;

using Cents = int64_t;

struct alignas(64) BankAccount {
    uint64_t account_id;
    Cents balance;
    mutex m;

    BankAccount(uint64_t id, Cents bal) : account_id(id), balance(bal) {}
};

enum class TransactionType {
    DEPOSIT,
    WITHDRAW,
    TRANSFER
};

struct TransactionTask {
    TransactionType type;
    BankAccount* from_account = nullptr;
    BankAccount* to_account = nullptr;
    Cents amount = 0;
    uint64_t transaction_id = 0;
};

bool deposit(BankAccount& account, Cents amount);
bool withdraw(BankAccount& account, Cents amount);
bool transfer(BankAccount& from, BankAccount& to, Cents amount);

Cents dollars_to_cents(double dollars);
double cents_to_dollars(Cents cents);

#endif