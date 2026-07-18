#include "bank.hpp"
#include <cmath>

bool deposit(BankAccount& account, Cents amount) {
    if (amount <= 0) return false;
    lock_guard<mutex> lock(account.m);
    account.balance += amount;
    return true;
}

bool withdraw(BankAccount& account, Cents amount) {
    if (amount <= 0) return false;
    lock_guard<mutex> lock(account.m);
    if (account.balance >= amount) {
        account.balance -= amount;
        return true;
    }
    return false;
}

bool transfer(BankAccount& from, BankAccount& to, Cents amount) {
    if (&from == &to || amount <= 0) return false;
    scoped_lock lock(from.m, to.m);
    if (from.balance >= amount) {
        from.balance -= amount;
        to.balance += amount;
        return true;
    }
    return false;
}

Cents dollars_to_cents(double dollars) {
    return static_cast<Cents>(llround(dollars * 100.0));
}

double cents_to_dollars(Cents cents) {
    return static_cast<double>(cents) / 100.0;
}