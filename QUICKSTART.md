# PebbleKV Quick Start Guide

## 1. Build

```batch
build_server.bat
```

This compiles:
- `pebblekv_server.exe` - The database server
- `pebblekv_client.exe` - Command-line client

## 2. Start Server

```batch
pebblekv_server.exe
```

You should see:
```
PebbleKV v0.5 - Phase 5: Expiration & LRU
==========================================

WAL enabled: data/pebblekv.wal
Memory limit enabled: 1000 keys max
PebbleKV server started on port 6379
Multi-client mode enabled (Phase 3)
Waiting for connections...
```

## 3. Connect Client

Open another terminal:

```batch
pebblekv_client.exe
```

## 4. Try Commands

```
pebblekv> SET name Alice
OK

pebblekv> GET name
Alice

pebblekv> SET age 30
OK

pebblekv> EXPIRE age 60
OK

pebblekv> TTL age
59 seconds

pebblekv> DELETE name
OK

pebblekv> GET name
(nil)

pebblekv> quit
Goodbye!
```

## 5. Test Persistence

```batch
REM Add some data
echo SET user1 Alice | pebblekv_client.exe
echo SET user2 Bob | pebblekv_client.exe

REM Kill server
taskkill /F /IM pebblekv_server.exe

REM Restart server
pebblekv_server.exe

REM Data should still be there!
echo GET user1 | pebblekv_client.exe
# Returns: Alice
```

## 6. Test Concurrency

Open 3 terminals and run client in each:

**Terminal 1:**
```
SET client1 Alice
GET client1
```

**Terminal 2:**
```
SET client2 Bob
GET client2
```

**Terminal 3:**
```
SET client3 Carol
GET client3
```

All clients can work simultaneously!

## 7. Run Tests

```batch
REM Phase 1: Basic operations
powershell -ExecutionPolicy Bypass -File test_phase1.ps1

REM Phase 2: Networking
test_phase2.bat

REM Phase 3: Concurrency
test_phase3.bat

REM Phase 4: Persistence
test_phase4.bat

REM Phase 5: TTL & LRU
test_phase5.bat
```

## 8. Verify Everything

```batch
verify_build.bat
```

This checks all components and runs a quick test.

## Common Commands

```
SET key value          Store key-value pair
GET key                Retrieve value
DELETE key             Remove key
EXPIRE key seconds     Set expiration
TTL key                Get time remaining
quit                   Disconnect
```

## Troubleshooting

### "Port already in use"
Kill existing server: `taskkill /F /IM pebblekv_server.exe`

### "Connection refused"
Make sure server is running first.

### Data not persisting
Check if `data/` folder exists. Server creates it automatically.

### Build fails
Make sure you have GCC 6.3+ or install Visual Studio Build Tools.

## What's Happening Behind the Scenes

1. **SET** → Logs to WAL → Stores in memory → Updates LRU
2. **GET** → Checks TTL → Returns value → Updates LRU
3. **DELETE** → Logs to WAL → Removes from memory
4. **EXPIRE** → Sets TTL timestamp
5. **On startup** → Replays WAL → Restores state

## Next Steps

- Read `COMPLETION_SUMMARY.md` for full project overview
- Read `docs/` for architecture details
- Try modifying the code
- Add your own features!

---

**Enjoy exploring PebbleKV!** 🚀
