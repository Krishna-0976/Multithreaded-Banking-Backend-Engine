#include "bank.hpp"

bool deposit(BankAccount& account, double amount) {
    if (amount <= 0) return false;
    std::lock_guard<std::mutex> lock(account.m);
    account.balance += amount;
    return true;
}

bool withdraw(BankAccount& account, double amount) {
    if (amount <= 0) return false;
    std::lock_guard<std::mutex> lock(account.m);
    if (account.balance >= amount) {
        account.balance -= amount;
        return true;
    }
    return false;
}

bool transfer(BankAccount& from, BankAccount& to, double amount) {
    if (&from == &to || amount <= 0) return false;

    std::scoped_lock lock(from.m, to.m);

    if (from.balance >= amount) {
        from.balance -= amount;
        to.balance += amount;
        return true;
    }
    return false;
}