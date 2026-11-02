# 🚀 KV Cache Engine

A lightweight, high-performance Redis alternative built from scratch in C++.

## ✨ Features
- **Multi-threaded TCP Server**: Handles concurrent client connections.
- **Custom RESP Protocol**: Parses Redis Serialization Protocol directly over raw sockets.
- **Thread-Safe Storage**: Uses `std::shared_mutex` for high-performance reads and safe writes.
- **AOF Persistence**: Append-Only File system to recover data after a crash.
- **TTL Support**: Automatic key expiration (EXPIRE, TTL).
- **Snapshots**: Save database state to disk manually (SAVE).

## 🛠️ Tech Stack
- **Language:** C++17
- **Networking:** POSIX Sockets (`sys/socket.h`)
- **Concurrency:** `std::thread`, `std::shared_mutex`
- **Build System:** CMake

## 🚦 How to Build & Run
*(Note: Requires a Linux/Mac environment or WSL on Windows)*
```bash
mkdir build && cd build
cmake ..
make
./kv_server 6379