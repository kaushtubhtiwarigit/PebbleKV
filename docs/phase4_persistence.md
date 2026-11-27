# Phase 4: Write-Ahead Log (WAL)

## Overview

Phase 4 adds **durability** to PebbleKV. Data now survives server restarts and crashes through a Write-Ahead Log.

## The Problem

Without persistence:
- Server crashes → all data lost
- Server restart → empty database
- No way to recover from failures

This makes the database **useless for real applications**.

## The Solution: Write-Ahead Log (WAL)

A WAL is an append-only log file that records every write operation **before** applying it to the in-memory database.

### How It Works

```
Client sends: SET name John

1. Write to WAL:  "SET name John\n"  ← Write to disk FIRST
2. fsync/flush                        ← Ensure it's on disk
3. Update memory: store_["name"] = "John"
4. Send response: "OK"
```

**Key principle:** Log to disk BEFORE updating memory. If we crash after step 1-2 but before step 3, we can replay the log and recover.

### Recovery Process

On server startup:
1. Check if WAL file exists
2. If exists, replay all operations from the log
3. Reconstruct in-memory state
4. Continue normal operation (appending to log)

```
Startup:
  Read WAL: "SET name John\n"
           "SET age 25\n"
           "DEL age\n"
           
  Apply:   store_["name"] = "John"
          store_["age"] = "25"
          store_.erase("age")
          
  Result:  Database restored to last state
```

## Implementation

### WAL Class (`wal.h`, `wal.cpp`)

**Responsibilities:**
- Open log file in append mode
- Write SET and DELETE operations
- Flush to disk immediately (durability)

**Format:**
```
SET key value
DEL key
```

Simple text format:
- Easy to debug (can read with text editor)
- Easy to parse
- Human-readable

### WALReader Class

**Responsibilities:**
- Read log file line by line
- Parse commands
- Used during recovery

### Database Integration

Modified `Database` class:
- `enableWAL()` - Start logging writes
- `replayWAL()` - Restore from log
- All `set()` and `del()` operations write to WAL first

### Crash Safety

**What happens on crash?**

| Timing | WAL | Memory | Result |
|--------|-----|--------|--------|
| Before WAL write | ❌ Not written | ❌ Not updated | Operation lost (client sees error) |
| After WAL write | ✅ On disk | ❌ Not updated | Operation recovered on restart |
| After memory update | ✅ On disk | ✅ Updated | Perfect - both saved |

**Key insight:** Once WAL write succeeds, operation is durable even if we crash immediately after.

### Performance Considerations

**Cost of durability:**
- Each write operation requires disk I/O
- Much slower than pure in-memory
- `fsync()` / `flush()` adds latency

**Mitigation strategies (not implemented yet):**
- Batch writes (group multiple operations)
- Async fsync (trade durability for speed)
- Write buffer (flush periodically)

**Current approach:** Flush immediately after every write (maximum safety, lower performance).

## File Format

```
SET username alice
SET email alice@example.com
SET age 30
DEL age
SET age 31
```

**Why not binary?**
- Text is debuggable
- Easy to implement correctly
- Performance difference is negligible for educational purposes
- Production systems (PostgreSQL, MySQL) use binary WAL for efficiency

## Recovery Guarantees

**What we guarantee:**
- All operations that returned "OK" are durable
- Database can survive process crash
- Database can survive system crash (if OS fsync works)

**What we don't guarantee:**
- Protection against disk failure (need replication)
- Protection against file corruption (need checksums)
- Atomic multi-key operations (need transactions)

## Limitations

**Current implementation:**
- WAL grows unbounded (no compaction/truncation)
- No checksums (can't detect corruption)
- Synchronous writes (slower than async)
- No log rotation
- Replays entire log on startup (slow for large logs)

**Production systems solve these with:**
- Log compaction/truncation after snapshots
- Checksums on every entry
- Group commit (batch fsync)
- Incremental recovery
- Log-structured merge trees (LSM trees)

## Testing

### Manual Test

```batch
REM Start server
pebblekv_server.exe

REM In another terminal, add data
pebblekv_client.exe
SET name John
GET name
quit

REM Kill server (simulate crash)
taskkill /F /IM pebblekv_server.exe

REM Restart server
pebblekv_server.exe

REM Connect again
pebblekv_client.exe
GET name
# Should return: John
```

### Automated Test

```batch
test_phase4.bat
```

Adds data, kills server, restarts, verifies data survived.

## Code Flow

### Write Path

```
Client → Server → Database.set(key, value)
                      ↓
                  WAL.logSet(key, value)
                      ↓
                  Write "SET key value\n" to file
                      ↓
                  Flush to disk
                      ↓
                  Update store_[key] = value
                      ↓
                  Return OK
```

### Startup Path

```
Server starts
    ↓
Database.replayWAL("data/pebblekv.wal")
    ↓
WALReader opens file
    ↓
For each line:
    Parse command
    If SET: store_[key] = value
    If DEL: store_.erase(key)
    ↓
WALReader closes file
    ↓
Database.enableWAL("data/pebblekv.wal")
    ↓
WAL opens file in append mode
    ↓
Server ready (data restored)
```

## Concurrency

WAL writes are protected by the database mutex:
- Only one thread writes to WAL at a time
- Safe for multi-client access
- Serializes all write operations

**Lock order:**
1. Acquire database mutex
2. Write to WAL
3. Update in-memory store
4. Release mutex

This prevents:
- Concurrent WAL writes (corruption)
- Race between WAL and memory
- Inconsistent state

## Real-World Examples

**Redis:** Uses AOF (Append-Only File) - same concept as our WAL
**PostgreSQL:** Uses WAL for crash recovery and replication
**MySQL InnoDB:** Uses redo log (similar to WAL)
**SQLite:** Uses WAL mode for better concurrency

Our implementation is simplified but follows the same principles.

## What We Learned

1. **Durability requires disk I/O** - cannot avoid the cost
2. **Write-ahead is key** - log before modifying state
3. **Flush is critical** - data not durable until fsynced
4. **Recovery is replay** - reconstruct state from log
5. **Simple format works** - text-based WAL is fine

## Next Steps

**Phase 5** will add:
- Key expiration (TTL)
- LRU eviction (memory limits)

**Phase 6** will add:
- Snapshotting (compact WAL)
- Faster recovery (snapshot + partial replay)

---

**Phase 4 complete!** PebbleKV now survives crashes and restarts.
