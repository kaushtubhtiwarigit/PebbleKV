# PebbleKV - Complete Implementation

A fully functional key-value database built from scratch in C++.

## ✅ All Phases Complete

### Phase 1: Basic Storage
- In-memory hash table (std::unordered_map)
- SET, GET, DELETE operations
- O(1) average-case performance

### Phase 2: TCP Networking
- TCP server on port 6379
- Text-based protocol
- Multiple sequential clients

### Phase 3: Concurrency
- Multi-threaded client handling
- Mutex-protected shared state
- Thread-safe operations

### Phase 4: Persistence
- Write-Ahead Log (WAL)
- Crash recovery
- Data survives restarts

### Phase 5: Memory Management
- TTL (Time-To-Live) expiration
- LRU (Least Recently Used) eviction
- Configurable memory limits

## Commands

```
SET key value          - Store key-value pair
GET key                - Retrieve value
DELETE key             - Remove key
EXPIRE key seconds     - Set expiration time
TTL key                - Get remaining time
```

## Build & Run

```batch
REM Build
build_server.bat

REM Start server
pebblekv_server.exe

REM Connect client (another terminal)
pebblekv_client.exe
```

## Architecture

```
Client → TCP → Server Thread → Mutex → Database
                                         ↓
                                        WAL (Disk)
                                         ↓
                                    LRU + TTL
```

## Key Features

- **Fast**: O(1) hash table lookups
- **Durable**: Write-ahead logging
- **Concurrent**: Thread-safe multi-client
- **Smart**: Automatic expiration & eviction
- **Simple**: ~3000 lines of readable C++

## Files

```
include/
  database.h          - Core storage
  server.h            - TCP server
  wal.h               - Persistence
  lru_cache.h         - Eviction
  command_parser.h    - Protocol
  thread_compat.h     - Threading

src/
  *.cpp               - Implementations

tests/
  test_*.bat          - Integration tests

docs/
  phase*.md           - Architecture docs
```

## Testing

```batch
test_phase1.ps1  - Basic operations
test_phase2.bat  - Networking
test_phase3.bat  - Concurrency
test_phase4.bat  - Persistence
test_phase5.bat  - TTL & LRU
```

## What You Learned

1. **Data Structures**: Hash tables, LRU caches, linked lists
2. **Networking**: Sockets, TCP/IP, protocols
3. **Concurrency**: Threads, mutexes, race conditions
4. **Persistence**: WAL, crash recovery, durability
5. **Systems Design**: Tradeoffs, performance, correctness

## Production Considerations

This is an **educational** database. Production systems need:

- Binary protocol (efficiency)
- Replication (high availability)
- Sharding (horizontal scaling)
- Transactions (ACID guarantees)
- Monitoring & metrics
- Security & authentication
- Advanced data structures (skip lists, B-trees)
- Async I/O (epoll, io_uring)

## Real-World Examples

- **Redis**: Similar architecture, AOF = our WAL
- **Memcached**: Pure in-memory, no persistence
- **RocksDB**: LSM-tree instead of hash table
- **SQLite**: B-tree with MVCC

## Interview Ready

You can now confidently explain:
- How databases store data
- How persistence works (WAL)
- How caching works (LRU)
- How servers handle concurrency
- Real-world tradeoffs

## License

MIT - Educational use encouraged

---

**Congratulations!** You built a working database from scratch.
