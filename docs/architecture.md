# PebbleKV Architecture

## Phase 1: Basic Key-Value Storage

### Overview

Phase 1 implements a simple in-memory key-value store with a command-line interface.

### Components

#### 1. Database (`database.h`, `database.cpp`)

**Purpose:** Core storage engine

**Data Structure:** `std::unordered_map<std::string, std::string>`

**Why unordered_map?**
- Hash table implementation
- O(1) average case for insert, lookup, delete
- Well-tested STL container
- Automatic memory management

**Key Methods:**
- `set()`: Insert or update a key-value pair
- `get()`: Retrieve value by key (returns `std::optional`)
- `del()`: Delete a key-value pair
- `exists()`: Check if key exists
- `size()`: Get number of keys
- `clear()`: Remove all data

**Thread Safety:** None (single-threaded in Phase 1). Will add mutex in Phase 3.

#### 2. Command Parser (`command_parser.h`, `command_parser.cpp`)

**Purpose:** Parse text commands into structured data

**Input:** Raw string like `"SET name John"`

**Output:** `Command` struct with:
- `type`: Enum indicating command type (SET, GET, DELETE, UNKNOWN)
- `args`: Vector of arguments

**Features:**
- Case-insensitive command matching
- Whitespace handling
- Support for DEL as alias for DELETE

**Design Choice:** Keep parsing simple. Advanced features like quoted strings with spaces will come later if needed.

#### 3. Main CLI (`main.cpp`)

**Purpose:** Interactive command-line interface for testing

**Flow:**
1. Create Database instance
2. Read line from stdin
3. Parse command
4. Execute command on database
5. Print response
6. Repeat

**Command Execution:**
- SET: Requires 2+ args (key, value). Multi-word values are joined with spaces.
- GET: Requires 1 arg (key). Returns value or "(nil)".
- DELETE: Requires 1 arg (key). Returns "OK" or "(nil)".

**Error Handling:** Invalid commands return error messages. Missing arguments are caught.

### Data Flow

```
User Input: "SET name John"
     ↓
CommandParser::parse()
     ↓
Command { type=SET, args=["name", "John"] }
     ↓
executeCommand()
     ↓
db.set("name", "John")
     ↓
unordered_map stores {"name": "John"}
     ↓
Response: "OK"
```

### Memory Layout

```
┌─────────────────────────────────────┐
│          Database Object            │
│                                     │
│  store_ (unordered_map)             │
│  ┌────────────────────────────┐    │
│  │  Bucket Array (hash table) │    │
│  │                            │    │
│  │  "name"  → "John"          │    │
│  │  "age"   → "25"            │    │
│  │  "city"  → "San Francisco" │    │
│  └────────────────────────────┘    │
└─────────────────────────────────────┘
```

**Memory Characteristics:**
- Hash table grows dynamically
- Each entry stores two strings (key + value)
- Typical overhead: ~24 bytes per entry (pointers, hash, metadata)
- No memory limits in Phase 1

### Testing Strategy

**Unit Tests** (`test_database.cpp`):
1. **Functionality tests:** Verify SET, GET, DELETE work correctly
2. **Edge cases:** Empty strings, non-existent keys, updates
3. **State tests:** Size tracking, clear operation
4. **Parser tests:** Case handling, whitespace, unknown commands

**Coverage:**
- All public methods tested
- Both success and failure paths
- Edge cases like empty keys/values

### Performance Characteristics (Phase 1)

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| SET       | O(1) average   | O(key + value)   |
| GET       | O(1) average   | O(1)             |
| DELETE    | O(1) average   | O(1)             |

**Notes:**
- Worst case is O(n) if all keys hash to same bucket (extremely rare)
- No persistence overhead in Phase 1
- No network overhead in Phase 1

### Limitations and Future Work

**Current Limitations:**
1. **No persistence:** Data lost when program exits
2. **No networking:** Only local CLI access
3. **No concurrency:** Can't handle multiple clients
4. **No memory limits:** Can exhaust system memory
5. **No expiration:** Keys never expire
6. **Simple parsing:** Can't handle values with leading/trailing spaces

**How Future Phases Address These:**

- **Phase 2 (TCP Server):** Add networking, protocol handling
- **Phase 3 (Concurrency):** Add mutex protection, thread pool
- **Phase 4 (Persistence):** Add Write-Ahead Log, crash recovery
- **Phase 5 (Expiration):** Add TTL tracking, LRU eviction
- **Phase 6 (Snapshots):** Add periodic state dumps
- **Phase 7 (Benchmarks):** Measure and optimize performance

### Design Principles Applied

1. **KISS (Keep It Simple):** No premature optimization, clear code over clever code
2. **RAII:** Let STL containers manage memory
3. **Type Safety:** Use `std::optional` instead of magic values
4. **Testability:** Pure functions, dependency injection ready
5. **Separation of Concerns:** Parser, storage, and UI are independent

### Questions This Phase Answers

- How do databases store data in memory?
- What data structure is best for key-value storage?
- How do you parse user commands?
- How do you handle errors gracefully?
- What does O(1) lookup really mean?

---

*Next: Phase 2 will add TCP networking and explore client-server architecture.*
