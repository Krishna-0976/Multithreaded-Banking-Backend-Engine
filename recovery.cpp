#include "recovery.hpp"
#include <fstream>
#include <sstream>

namespace {

const char* type_to_string(TransactionType t) {
    switch (t) {
        case TransactionType::DEPOSIT:  return "DEPOSIT";
        case TransactionType::WITHDRAW: return "WITHDRAW";
        case TransactionType::TRANSFER: return "TRANSFER";
    }
    return "UNKNOWN";
}

bool string_to_type(const string& s, TransactionType& out) {
    if (s == "DEPOSIT")  { out = TransactionType::DEPOSIT;  return true; }
    if (s == "WITHDRAW") { out = TransactionType::WITHDRAW; return true; }
    if (s == "TRANSFER") { out = TransactionType::TRANSFER; return true; }
    return false;
}

}

string format_ledger_line(uint64_t transaction_id, TransactionType type,
                           uint64_t from_id, uint64_t to_id,
                           Cents amount, bool success) {
    ostringstream oss;
    oss << transaction_id << '|'
        << type_to_string(type) << '|'
        << from_id << '|'
        << to_id << '|'
        << amount << '|'
        << (success ? "SUCCESS" : "FAILED");
    return oss.str();
}

bool parse_ledger_line(const string& line, LedgerEntry& out) {
    istringstream iss(line);
    string field;
    vector<string> fields;
    fields.reserve(6);

    while (getline(iss, field, '|')) {
        fields.push_back(field);
    }
    if (fields.size() != 6) return false;

    try {
        out.transaction_id = stoull(fields[0]);
        if (!string_to_type(fields[1], out.type)) return false;
        out.from_id = stoull(fields[2]);
        out.to_id = stoull(fields[3]);
        out.amount = static_cast<Cents>(stoll(fields[4]));
        if (fields[5] == "SUCCESS") out.success = true;
        else if (fields[5] == "FAILED") out.success = false;
        else return false;
    } catch (...) {
        return false;
    }
    return true;
}

vector<LedgerEntry> load_ledger(const string& path) {
    vector<LedgerEntry> entries;
    ifstream file(path);
    if (!file.is_open()) return entries;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        LedgerEntry entry{};
        if (parse_ledger_line(line, entry)) {
            entries.push_back(entry);
        }
    }
    return entries;
}

void replay_ledger(unordered_map<uint64_t, BankAccount*>& accounts_by_id,
                    const vector<LedgerEntry>& entries) {
    for (const auto& entry : entries) {
        if (!entry.success) continue;

        switch (entry.type) {
            case TransactionType::DEPOSIT: {
                auto it = accounts_by_id.find(entry.to_id);
                if (it != accounts_by_id.end()) {
                    it->second->balance += entry.amount;
                }
                break;
            }
            case TransactionType::WITHDRAW: {
                auto it = accounts_by_id.find(entry.to_id);
                if (it != accounts_by_id.end()) {
                    it->second->balance -= entry.amount;
                }
                break;
            }
            case TransactionType::TRANSFER: {
                auto from_it = accounts_by_id.find(entry.from_id);
                auto to_it = accounts_by_id.find(entry.to_id);
                if (from_it != accounts_by_id.end()) {
                    from_it->second->balance -= entry.amount;
                }
                if (to_it != accounts_by_id.end()) {
                    to_it->second->balance += entry.amount;
                }
                break;
            }
        }
    }
}