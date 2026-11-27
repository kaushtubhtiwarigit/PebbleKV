# PebbleKV

A lightweight, educational key-value database built from scratch in C++20.

## Project Overview

PebbleKV is designed to help understand how databases work internally. It's not production-ready but implements core database concepts in a clear, understandable way.

## Current Status: All Phases Complete! ✅

**Implemented Features:**
- ✅ In-memory key-value storage (hash table)
- ✅ SET, GET, DELETE operations
- ✅ **TCP Server - Network access**
- ✅ **Multi-client concurrency (threads + mutex)**
- ✅ **Write-Ahead Log - Crash recovery**
- ✅ **Key expiration (TTL)**
- ✅ **LRU eviction (memory limits)**
- ✅ Comprehensive testing

PebbleKV is now a fully functional database with persistence, concurrency, and memory management!

## Architecture

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│   Client 1   │────▶│              │     │   Client 2   │
└──────────────┘     │  TCP Server  │◀────└──────────────┘
                     │  (Port 6379) │     (waits in queue)
                     └──────┬───────┘
                            │
                            ▼
                   ┌─────────────────┐
                   │ Command Parser  │
                   └────────┬─────────┘
                            │
                            ▼
                   ┌─────────────────┐
                   │    Database     │
                   │  (unordered_map)│
                   └─────────────────┘
```

## Building PebbleKV

### Prerequisites

- CMake 3.14 or higher
- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- Internet connection (for downloading GoogleTest)

### Build Instructions

#### Quick Build (Windows with MinGW)

```batch
REM Build server and client
build_server.bat
```

This creates:
- `pebblekv_server.exe` - TCP server
- `pebblekv_client.exe` - TCP client  
- `pebblekv.exe` - Original CLI (Phase 1)

#### Manual Build

```batch
REM Server
g++ -std=c++14 -Wall -Wextra -I.\include -o pebblekv_server.exe .\src\main.cpp .\src\server.cpp .\src\database.cpp .\src\command_parser.cpp -lws2_32

REM Client
g++ -std=c++14 -Wall -Wextra -o pebblekv_client.exe .\src\client.cpp -lws2_32
```

#### With CMake (if installed)

```bash
mkdir build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

## Usage

### TCP Server (Phase 2)

**Start the server:**
```batch
pebblekv_server.exe
REM Or: run_server.bat
```

Output:
```
PebbleKV v0.2 - Phase 2: TCP Server
====================================

PebbleKV server started on port 6379
Waiting for connections...
```

**Connect with client:**
```batch
REM In another terminal
pebblekv_client.exe
REM Or: run_client.bat
```

**Example session:**
```
Connected to PebbleKV at 127.0.0.1:6379
Type commands (or 'quit' to exit)

pebblekv> SET name Kaushtubh
OK
pebblekv> GET name
Kaushtubh
pebblekv> SET age 25
OK
pebblekv> GET age
25
pebblekv> DELETE age
OK
pebblekv> GET age
(nil)
pebblekv> quit
Goodbye!
```

**Connect from any TCP client:**
```batch
REM Using telnet
telnet localhost 6379

REM Using netcat
nc localhost 6379
```

### Local CLI (Phase 1)

```batch
pebblekv.exe
```

## Commands

### SET key value
Store a key-value pair. If the key exists, update it.
- Returns: `OK`

### GET key
Retrieve the value for a key.
- Returns: The value, or `(nil)` if key doesn't exist or expired

### DELETE key (or DEL)
Remove a key-value pair.
- Returns: `OK` if deleted, `(nil)` if key didn't exist

### EXPIRE key seconds
Set expiration time for a key.
- Returns: `OK` if key exists, error otherwise
- Key automatically deleted after specified seconds

### TTL key
Get remaining time-to-live for a key.
- Returns: Seconds remaining, "No expiration", or "Key not found"

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

## Known Limitations

- **Fixed memory limit** - 1000 keys default (configurable in code)
- **Simple LRU** - No approximate LRU optimizations
- **No log compaction** - WAL grows unbounded
- **No authentication** - Anyone can connect
- **Text protocol** - Binary would be faster
- **Windows-focused** - Linux support via POSIX compile

These are deliberate simplifications for educational clarity.

## Contributing

This is an educational project. Focus is on clarity and correctness, not production features.

## License

MIT License - See LICENSE file for details

## Author

Built as a learning project to understand database internals.

---

**Next Steps:** Complete Phase 2 to add TCP server support!
