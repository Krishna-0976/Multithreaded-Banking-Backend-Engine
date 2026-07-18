#ifndef RECOVERY_HPP
#define RECOVERY_HPP

#include "bank.hpp"
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct LedgerEntry {
    uint64_t transaction_id;
    TransactionType type;
    uint64_t from_id;
    uint64_t to_id;
    Cents amount;
    bool success;
};

bool parse_ledger_line(const string& line, LedgerEntry& out);

string format_ledger_line(uint64_t transaction_id, TransactionType type,
                           uint64_t from_id, uint64_t to_id,
                           Cents amount, bool success);

vector<LedgerEntry> load_ledger(const string& path);

void replay_ledger(unordered_map<uint64_t, BankAccount*>& accounts_by_id,
                    const vector<LedgerEntry>& entries);

#endif