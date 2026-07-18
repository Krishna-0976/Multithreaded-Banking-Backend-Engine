# 🏦 Multithreaded Banking Backend Engine

A high-performance concurrent banking backend built in **C++17** that safely processes thousands of simultaneous financial transactions using thread pooling, deadlock-free multi-account locking, asynchronous write-ahead logging, and ledger-replay crash recovery.

Built to demonstrate practical **Concurrency, Operating Systems, Data Structures & Algorithms, and Fault-Tolerant Systems Design** concepts on top of modern C++.

---

## 🎯 Project Objectives

- Process thousands of concurrent transactions with zero data races, deadlocks, or lost funds.
- Eliminate race conditions and deadlocks in shared multi-account operations.
- Minimize transaction latency via a reusable worker thread pool and asynchronous logging.
- Persist a durable, replayable record of every transaction (a write-ahead log).
- **Actually reconstruct account state from that log** — not just log to a file, but prove the log is sufficient to rebuild the system after a simulated crash.

---

## 🚀 Key Features

### ⚡ Fixed-Size Reusable Thread Pool
- Pre-spawns worker threads once at startup; no per-transaction thread creation.
- Workers pull from a shared bounded queue until it's drained and shut down.

### 🔒 Thread-Safe Bounded Transaction Queue
- Circular buffer protected by a mutex + two condition variables (`not_empty` / `not_full`).
- Blocks producers when full, blocks consumers when empty, and unblocks all waiters cleanly on `shutdown()`.

### 🛡 Deadlock-Free Fund Transfers
- Concurrent transfers between arbitrary account pairs use `std::scoped_lock(from.m, to.m)`, which acquires both mutexes using the standard library's built-in deadlock-avoidance algorithm — safe regardless of lock order or which thread reaches which account first.
- Balance checks happen after both locks are held, so transfers with insufficient funds are rejected without any partial effect.

### 📝 Asynchronous Write-Ahead Logging (WAL)
- Every transaction attempt (success or failure) is appended to `ledger.txt` in a compact, machine-parseable format: `tx_id|TYPE|from_id|to_id|amount_cents|STATUS`.
- A dedicated logger thread owns all file I/O, so worker threads never block on disk.

### 🔄 Crash Recovery (real, not aspirational)
- `recovery.{hpp,cpp}` parses the ledger back into structured entries and replays every `SUCCESS` entry onto a fresh set of accounts.
- `main.cpp` demonstrates this end-to-end: it runs the live stress test, then throws away the in-memory account state, rebuilds a second set of accounts from only their starting balances and `ledger.txt`, and asserts the recovered balances match the live ones exactly.
- Malformed or truncated lines (as you'd expect from a real crash mid-write) are skipped safely rather than crashing recovery.

### 💰 Integer-Cents Money Handling
- All balances and transaction amounts are `int64_t` cents (`Cents` type alias), never `double`. Floating-point currency accumulates rounding error under volume; integer cents give exact arithmetic. Dollar amounts only exist at the I/O boundary (`dollars_to_cents` / `cents_to_dollars`).

### 📊 Lock-Free Integrity Auditing
- A single `std::atomic<int64_t> global_net_change` tracks the bank's net inflow/outflow via `fetch_add`/`fetch_sub` — genuine lock-free hardware atomics, not a CAS-retry loop.
- `main.cpp` compares the audited expected total against the sum of actual final balances for an exact-match integrity check.

---

## 🧠 Concurrency Concepts Demonstrated

| Concept                    | Implementation                                                     |
| -------------------------- | ------------------------------------------------------------------ |
| Thread Pooling             | Fixed reusable worker threads                                      |
| Producer-Consumer Pattern  | Bounded transaction queue                                          |
| Mutual Exclusion           | `std::mutex`                                                       |
| RAII-Based Lock Management | `std::scoped_lock` / `std::lock_guard`                              |
| Condition Synchronization  | `std::condition_variable`                                          |
| Deadlock Prevention        | `std::scoped_lock`'s built-in lock-order-agnostic acquisition       |
| Lock-Free Operations       | `std::atomic<int64_t>`                                              |
| Asynchronous Processing    | Dedicated logger thread                                            |
| Fault Tolerance            | Write-Ahead Logging                                                |
| Crash Recovery             | Ledger parsing + replay onto fresh state                           |

---

## 📂 Project Structure

```
BANKING_SYSTEM/
├── bank.hpp / bank.cpp                  # Account model, deposit/withdraw/transfer
├── queue.hpp / queue.cpp                # Thread-safe bounded transaction queue
├── thread_pool.hpp / thread_pool.cpp    # Worker pool, executes tasks, writes to ledger
├── logger.hpp / logger.cpp              # Asynchronous write-ahead logger
├── recovery.hpp / recovery.cpp          # Ledger parsing + replay (crash recovery)
├── main.cpp                             # Stress test + live crash-recovery demo
├── tests/
│   └── test_bank.cpp                    # Dependency-free unit + concurrency tests
├── .gitignore
├── LICENSE
└── README.md
```

---

## ▶️ Build & Run

### Engine + stress test + recovery demo
```bash
g++ -std=c++17 -Wall -pthread bank.cpp queue.cpp thread_pool.cpp logger.cpp recovery.cpp main.cpp -o bank_system
./bank_system
```

### Tests
```bash
g++ -std=c++17 -Wall -I. -pthread tests/test_bank.cpp bank.cpp queue.cpp recovery.cpp -o tests/test_bank
./tests/test_bank
```
14 test cases / 34 assertions covering deposit/withdraw/transfer edge cases, concurrent transfers (money-conservation under contention), queue FIFO + shutdown behavior, and ledger format round-trip + replay correctness — including malformed-line handling and unknown-account-id safety.

---

## 🔬 Stress Test Results (representative run)

| Metric                              | Result             |
| ------------------------------------ | ------------------ |
| Transactions Processed               | 10,000             |
| Processing Time                      | ~15–25 ms          |
| Data Races                           | 0                  |
| Deadlocks                            | 0                  |
| Lost Funds                           | 0                  |
| Recovered State Matches Live State   | Yes (exact match)  |

Run it yourself — the numbers above will vary slightly by machine, but the integrity and recovery checks should always report `SUCCESS`.

---

## 🎯 Learning Outcomes

This project explores concepts found in banking platforms, payment processors, trading engines, and database systems: concurrent programming, OS-level synchronization, deadlock prevention, durable logging, and fault-tolerant recovery design.

---

👨‍💻 **Author**

**Krishna Parmar**
B.Tech ICT Student, Dhirubhai Ambani University
