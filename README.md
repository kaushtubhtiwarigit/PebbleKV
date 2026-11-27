# PebbleKV

**A fully functional key-value database built from scratch in C++ with TCP networking, multi-client concurrency, write-ahead logging, and automatic memory management.**

## Project Overview

PebbleKV is a complete, working database that demonstrates how real databases work internally. Built from first principles with ~3000 lines of clean, educational C++ code.

## 🎉 Status: 100% COMPLETE - All 5 Phases Implemented!

### Phase 1: Basic Storage ✅
- In-memory hash table (std::unordered_map)
- O(1) SET, GET, DELETE operations
- Interactive CLI with command parser

### Phase 2: TCP Networking ✅
- TCP server on port 6379
- Text-based protocol (like Redis)
- Multiple client support
- Cross-platform (Windows/Linux)

### Phase 3: Concurrency ✅
- Multi-threaded client handling
- Thread-safe database operations
- Mutex-protected shared state
- Windows native threading support

### Phase 4: Persistence (WAL) ✅
- Write-Ahead Log for durability
- Automatic crash recovery
- Log replay on startup
- Data survives restarts

### Phase 5: Memory Management ✅
- TTL (Time-To-Live) key expiration
- LRU (Least Recently Used) eviction
- Configurable memory limits (1000 keys default)
- EXPIRE and TTL commands

**Result:** A production-quality educational database that actually works!

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   Multiple Clients                      │
│            (Concurrent TCP connections)                 │
└──────────────────────┬──────────────────────────────────┘
                       │ TCP/IP (port 6379)
┌──────────────────────▼──────────────────────────────────┐
│              Multi-threaded TCP Server                  │
│           (One thread per client + main)                │
└──────────────────────┬──────────────────────────────────┘
                       │ Command Parser
┌──────────────────────▼──────────────────────────────────┐
│                   Mutex Lock                            │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│              Database (Thread-Safe)                     │
│    ┌──────────────────────────────────────┐            │
│    │  Hash Table (unordered_map)          │            │
│    │  - O(1) average operations           │            │
│    └──────────────────────────────────────┘            │
│    ┌──────────────────────────────────────┐            │
│    │  LRU Cache                            │            │
│    │  - Eviction policy (1000 keys max)   │            │
│    └──────────────────────────────────────┘            │
│    ┌──────────────────────────────────────┐            │
│    │  TTL Manager                          │            │
│    │  - Automatic expiration tracking     │            │
│    └──────────────────────────────────────┘            │
└──────────────────────┬──────────────────────────────────┘
                       │ Write-Ahead
┌──────────────────────▼──────────────────────────────────┐
│              Write-Ahead Log (WAL)                      │
│         data/pebblekv.wal (Persistent)                  │
└─────────────────────────────────────────────────────────┘
```

## Building PebbleKV

### Prerequisites

- CMake 3.14 or higher
- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- Internet connection (for downloading GoogleTest)

### Build Instructions

#### Windows (MinGW)

**Automated:**
```batch
build_server.bat
```

**Manual:**
```batch
g++ -std=c++14 -Wall -Wextra -I.\include ^
    -o pebblekv_server.exe ^
    .\src\main.cpp .\src\server.cpp .\src\database.cpp ^
    .\src\command_parser.cpp .\src\thread_compat.cpp ^
    .\src\wal.cpp .\src\lru_cache.cpp ^
    -lws2_32

g++ -std=c++14 -Wall -Wextra ^
    -o pebblekv_client.exe .\src\client.cpp ^
    -lws2_32
```

### Linux

```bash
g++ -std=c++14 -Wall -Wextra -I./include \
    -o pebblekv_server \
    src/main.cpp src/server.cpp src/database.cpp \
    src/command_parser.cpp src/thread_compat.cpp \
    src/wal.cpp src/lru_cache.cpp \
    -lpthread

g++ -std=c++14 -Wall -Wextra \
    -o pebblekv_client src/client.cpp
```

### With CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

**Output:**
- `pebblekv_server.exe` (or `pebblekv_server`) - Database server
- `pebblekv_client.exe` (or `pebblekv_client`) - Command-line client

## Usage

### Quick Start

**1. Build:**
```batch
build_server.bat
```

**2. Start Server:**
```batch
pebblekv_server.exe
```

Output:
```
PebbleKV v0.5 - Phase 5: Expiration & LRU
==========================================

Replaying WAL: data/pebblekv.wal
Replayed 0 operations from WAL
WAL enabled: data/pebblekv.wal
Memory limit enabled: 1000 keys max
PebbleKV server started on port 6379
Multi-client mode enabled (Phase 3)
Waiting for connections...
Waiting for connections...
```

**3. Connect Client (new terminal):**
```batch
pebblekv_client.exe
```

**4. Run Commands:**
```
Connected to PebbleKV at 127.0.0.1:6379
Type commands (or 'quit' to exit)

pebblekv> SET user:1 Alice
OK

pebblekv> SET user:2 Bob
OK

pebblekv> GET user:1
Alice

pebblekv> EXPIRE user:2 60
OK

pebblekv> TTL user:2
59 seconds

pebblekv> DELETE user:1
OK

pebblekv> GET user:1
(nil)

pebblekv> quit
Goodbye!
```

### Advanced Usage

**Multiple Clients:**
Open multiple terminals and run `pebblekv_client.exe` in each - they all work simultaneously!

**Using Telnet:**
```bash
telnet localhost 6379
SET test hello
GET test
quit
```

**Using Netcat:**
```bash
nc localhost 6379
SET test hello
GET test
quit
```

**Scripted Commands:**
```batch
echo SET key value | pebblekv_client.exe
echo GET key | pebblekv_client.exe
```

## Commands Reference

| Command | Syntax | Description | Example |
|---------|--------|-------------|---------|
| **SET** | `SET key value` | Store key-value pair (creates or updates) | `SET name Alice` |
| **GET** | `GET key` | Retrieve value for key | `GET name` → `Alice` |
| **DELETE** | `DELETE key` or `DEL key` | Remove key-value pair | `DELETE name` → `OK` |
| **EXPIRE** | `EXPIRE key seconds` | Set expiration time (TTL) | `EXPIRE session 300` |
| **TTL** | `TTL key` | Get remaining lifetime in seconds | `TTL session` → `245 seconds` |

### Response Format
- **Success:** `OK` or the requested value
- **Not Found:** `(nil)`
- **Error:** `ERROR: <message>`
- **TTL Special:** `No expiration` (no TTL set), `Key not found or expired` (doesn't exist)

## Project Structure

```
PebbleKV/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── build_server.bat         # Quick build script (Windows)
├── run_server.bat           # Start server
├── run_client.bat           # Start client
├── include/
│   ├── database.h           # Database with TTL & LRU
│   ├── server.h             # Multi-threaded TCP server
│   ├── wal.h                # Write-ahead logging
│   ├── lru_cache.h          # LRU eviction & TTL manager
│   ├── command_parser.h     # Command parsing
│   └── thread_compat.h      # Cross-platform threading
├── src/
│   ├── main.cpp             # Server entry point
│   ├── server.cpp           # TCP server implementation
│   ├── database.cpp         # Database implementation
│   ├── wal.cpp              # WAL implementation
│   ├── lru_cache.cpp        # LRU & TTL implementation
│   ├── command_parser.cpp   # Parser implementation
│   ├── client.cpp           # TCP client
│   └── thread_compat.cpp    # Threading helpers
├── tests/
│   ├── test_database.cpp    # Unit tests (GoogleTest)
│   ├── test_phase1.ps1      # Phase 1 tests
│   ├── test_phase2.bat      # Phase 2 tests
│   ├── test_phase3.bat      # Phase 3 tests
│   ├── test_phase4.bat      # Phase 4 tests
│   └── test_phase5.bat      # Phase 5 tests
├── data/                    # WAL files stored here
└── docs/
    ├── architecture.md      # Phase 1 design
    ├── phase2_networking.md # TCP architecture
    └── phase4_persistence.md# WAL architecture
```

## Testing

**Unit Tests:**
```bash
cd build
ctest --output-on-failure
```

**Integration Tests:**
```batch
test_phase1.ps1  # Basic operations
test_phase2.bat  # TCP networking  
test_phase3.bat  # Multi-client concurrency
test_phase4.bat  # Persistence & crash recovery
test_phase5.bat  # TTL expiration & LRU eviction
```

**Manual Testing:**
```batch
REM Terminal 1: Start server
pebblekv_server.exe

REM Terminal 2: Connect client
pebblekv_client.exe
SET mykey myvalue
EXPIRE mykey 60
TTL mykey
GET mykey
```

## Design Decisions

### Why `std::unordered_map`?
- O(1) average-case lookup, insert, and delete
- Simple and well-tested STL container
- Good enough for educational purposes

### Why `std::optional` for GET?
- Clearly distinguishes between "key exists with empty value" and "key doesn't exist"
- Type-safe alternative to returning empty string or throwing exceptions
- Modern C++ best practice

### Why separate parser?
- Easier to test independently
- Will be reused when we add TCP server
- Clean separation of concerns

## Known Limitations (By Design)

These are **intentional simplifications** for educational clarity:

| Limitation | PebbleKV | Production Solution |
|------------|----------|---------------------|
| Memory limit | Fixed 1000 keys | Dynamic configuration, monitoring |
| WAL size | Grows unbounded | Log compaction, rotation |
| Protocol | Text-based | Binary (RESP, Protobuf) |
| Authentication | None | ACLs, passwords, TLS |
| Data types | Strings only | Lists, sets, hashes, sorted sets |
| Clustering | Single node | Sharding, replication |
| Transactions | None | MULTI/EXEC, optimistic locking |
| Performance | Good for 100s clients | Async I/O for 10,000+ clients |

**PebbleKV demonstrates core concepts. Production databases add layers of optimization and features on top of these fundamentals.**

## Contributing

This is an educational project. Focus is on clarity and correctness, not production features.

## License

MIT License - See LICENSE file for details

## Author

Built as a learning project to understand database internals.

---

**Next Steps:** Complete Phase 2 to add TCP server support!
