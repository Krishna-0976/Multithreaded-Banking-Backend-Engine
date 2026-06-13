#ifndef BANK_HPP
#define BANK_HPP

#include <cstdint>
#include <mutex>

struct alignas(64) BankAccount {
    uint64_t account_id;
    double balance;
    std::mutex m;

    BankAccount(uint64_t id, double bal) : account_id(id), balance(bal) {}
};

enum class TransactionType {
    DEPOSIT,
    WITHDRAW,
    TRANSFER
};

struct TransactionTask {
    TransactionType type;
    BankAccount* from_account;
    BankAccount* to_account;
    double amount;
    uint64_t transaction_id;
};

bool deposit(BankAccount& account, double amount);
bool withdraw(BankAccount& account, double amount);
bool transfer(BankAccount& from, BankAccount& to, double amount);

#endif
