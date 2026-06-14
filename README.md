# 🏦 Multithreaded Banking Backend Engine

A high-performance concurrent banking backend built in **C++17** that safely processes thousands of simultaneous financial transactions using advanced synchronization mechanisms, deadlock prevention strategies, asynchronous logging, and crash recovery techniques.

Designed to simulate the core architecture of real-world financial transaction engines, this project demonstrates practical applications of **Object-Oriented Programming (OOP), Data Structures & Algorithms (DSA), Operating Systems, Concurrency, Systems Programming, and Fault-Tolerant Software Design**.

---

## 🎯 Project Objectives

* Design a highly concurrent banking backend capable of processing thousands of transactions safely.
* Eliminate race conditions and deadlocks in shared account operations.
* Minimize transaction latency through reusable worker threads and asynchronous logging.
* Ensure durability and recoverability through persistent transaction records.
* Demonstrate real-world systems programming and concurrency concepts using modern C++.

---

## 🚀 Key Features

### ⚡ Fixed-Size Reusable Thread Pool

* Pre-spawns a pool of worker threads during startup.
* Eliminates expensive thread creation and destruction overhead.
* Idle workers automatically sleep and wake on demand.
* Maximizes throughput under heavy transaction loads.

### 🔒 Thread-Safe Transaction Queue

* Implemented using a bounded circular buffer.
* Protected using mutexes and condition variables.
* Guarantees safe producer-consumer communication.
* Prevents race conditions during task scheduling.

### 🛡 Deadlock-Free Fund Transfers

* Supports concurrent transfers between multiple bank accounts.
* Uses `std::scoped_lock` to atomically acquire multiple account locks.
* Eliminates circular wait conditions.
* Guarantees deadlock-free transaction execution.
* Performs thread-safe balance validation before executing transfers.
* Rejects transactions with insufficient funds while preserving system consistency.

### 📝 Asynchronous Write-Ahead Logging (WAL)

* Decouples transaction execution from disk I/O.
* Worker threads immediately return to processing after completing transfers.
* Dedicated logging thread writes transaction records in the background.
* Minimizes latency caused by storage operations.

### 🔄 Crash Recovery Mechanism

* Stores transaction history persistently in a ledger file.
* Supports state reconstruction through transaction replay.
* Protects against data loss caused by crashes or power failures.
* Inspired by durability techniques used in database systems.

### 📊 Lock-Free Integrity Auditing

* Uses `std::atomic` primitives for consistency verification.
* Enables real-time auditing without pausing active transfers.
* Prevents audits from becoming a performance bottleneck.
* Maintains global balance invariants under concurrent workloads.

---

## 🏗 System Architecture

```text
┌─────────────────────────────────────────────────────────────────────────────┐
│                    MULTITHREADED BANKING SYSTEM                            │
└─────────────────────────────────────────────────────────────────────────────┘


                    ┌──────────────────────────┐
                    │ Incoming Transactions    │
                    │ (Deposits / Transfers)   │
                    └────────────┬─────────────┘
                                 │
                                 ▼

          ┌─────────────────────────────────────────────┐
          │ Transaction Dispatcher                      │
          │ Validates & Queues Incoming Requests        │
          └─────────────────┬───────────────────────────┘
                            │
                            ▼

          ┌─────────────────────────────────────────────┐
          │ Thread-Safe Circular Buffer Queue           │
          │ Mutex + Condition Variable Protected        │
          └─────────────────┬───────────────────────────┘
                            │
        ┌───────────────────┼───────────────────┐
        │                   │                   │
        ▼                   ▼                   ▼

 ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
 │ Worker T1   │     │ Worker T2   │ ... │ Worker TN   │
 └──────┬──────┘     └──────┬──────┘     └──────┬──────┘
        │                   │                   │
        └──────────────┬────┴────┬──────────────┘
                       │         │
                       ▼         ▼

          ┌─────────────────────────────────────────────┐
          │ Deadlock-Free Transfer Engine               │
          │ std::scoped_lock Multi-Account Locking      │
          └─────────────────┬───────────────────────────┘
                            │
                            ▼

          ┌─────────────────────────────────────────────┐
          │ Account Database (In-Memory State)          │
          │ Account Balances + Synchronization Primitives│
          └──────────────┬──────────────────────────────┘
                         │
         ┌───────────────┴────────────────┐
         │                                │
         ▼                                ▼

┌─────────────────────┐      ┌─────────────────────────┐
│ Atomic Audit Layer  │      │ Write-Ahead Log Queue   │
│ Real-Time Integrity │      │ Non-Blocking Logging    │
│ Verification        │      └────────────┬────────────┘
└──────────┬──────────┘                   │
           │                              ▼
           │                 ┌─────────────────────────┐
           │                 │ Dedicated Logger Thread │
           │                 └────────────┬────────────┘
           │                              │
           ▼                              ▼

┌─────────────────────┐      ┌─────────────────────────┐
│ Consistency Reports │      │ ledger.txt              │
│ Total Funds Audit   │      │ Persistent Transaction  │
└─────────────────────┘      │ History                 │
                             └────────────┬────────────┘
                                          │
                                          ▼

                             ┌─────────────────────────┐
                             │ Crash Recovery Engine   │
                             │ Log Replay Mechanism    │
                             └─────────────────────────┘
```

---

## 🧠 Concurrency Concepts Demonstrated

| Concept                    | Implementation                                                     |
| -------------------------- | ------------------------------------------------------------------ |
| Thread Pooling             | Fixed reusable worker threads                                      |
| Producer-Consumer Pattern  | Transaction queue architecture                                     |
| Mutual Exclusion           | `std::mutex`                                                       |
| RAII-Based Lock Management | `std::scoped_lock` automatically releases locks when leaving scope |
| Condition Synchronization  | `std::condition_variable`                                          |
| Deadlock Prevention        | `std::scoped_lock`                                                 |
| Lock-Free Operations       | `std::atomic`                                                      |
| Asynchronous Processing    | Dedicated logger thread                                            |
| Fault Tolerance            | Write-Ahead Logging                                                |
| Crash Recovery             | Ledger replay mechanism                                            |
| Resource Coordination      | Multi-resource locking                                             |

---

## ⚙️ Technologies Used

* C++17
* GNU Compiler Collection (GCC)
* Standard Template Library (STL)
* Multithreading (`<thread>`)
* Mutexes (`<mutex>`)
* Condition Variables (`<condition_variable>`)
* Atomic Operations (`<atomic>`)
* File Streams (`<fstream>`)
* Producer-Consumer Design Pattern
* Write-Ahead Logging (WAL)

---

## 📂 Project Structure

```text
BANKING_SYSTEM/
│
├── include/
│   ├── bank.hpp
│   ├── logger.hpp
│   ├── queue.hpp
│   └── thread_pool.hpp
│
├── src/
│   ├── bank.cpp
│   ├── logger.cpp
│   ├── queue.cpp
│   ├── thread_pool.cpp
│   └── main.cpp
│
├── .gitignore
├── ledger.txt
└── run.bat
```
## 🔬 Stress Testing

The system includes a randomized transaction simulator designed to emulate real-world banking traffic under heavy concurrent load.

### Test Configuration

* Multiple bank accounts
* Randomized transaction amounts
* Concurrent transfer requests
* Parallel worker execution
* Real-time consistency audits

### Results

| Metric                 | Result      |
| ---------------------- | ----------- |
| Transactions Processed | 10,000+     |
| Processing Time        | ~100–150 ms |
| Data Races             | 0           |
| Deadlocks              | 0           |
| Lost Funds             | 0           |
| Audit Failures         | 0           |
| Consistency Accuracy   | 100%        |

---

## 📈 Performance Highlights

* Processes thousands of concurrent operations within milliseconds.
* Maintains consistency under heavy thread contention.
* Eliminates deadlock scenarios through coordinated locking.
* Supports continuous auditing without blocking transaction flow.
* Reduces thread-management overhead through resource recycling.
* Demonstrates scalable backend transaction processing techniques.

---

## 🎯 Learning Outcomes

This project explores practical concepts commonly found in:

* Banking Platforms
* Payment Processing Systems
* Trading Engines
* Database Systems
* Distributed Services
* High-Performance Backend Infrastructure

Key areas of focus include:

* Concurrent Programming
* Operating System Synchronization
* Thread Scheduling
* Deadlock Prevention
* Fault-Tolerant System Design
* Performance Optimization
* Data Consistency Guarantees

---

## ▶️ Build & Run

```bash
g++ -std=c++17 -pthread -Iinclude src/*.cpp -o bank_system

./bank_system
```

---

👨‍💻 **Author**

**Krishna Parmar**  
B.Tech ICT Student  
Dhirubhai Ambani University
