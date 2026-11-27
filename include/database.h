#pragma once

#include "thread_compat.h"
#include "lru_cache.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace pebblekv {

class WAL;  // Forward declaration

// Simple wrapper for optional value (pre-C++17 compatibility)
// Use std::optional when available
template<typename T>
class Optional {
public:
    Optional() : has_value_(false) {}
    Optional(const T& value) : value_(value), has_value_(true) {}
    
    bool has_value() const { return has_value_; }
    const T& value() const { return value_; }
    const T& operator*() const { return value_; }
    explicit operator bool() const { return has_value_; }
    
private:
    T value_;
    bool has_value_;
};

// Simple in-memory key-value store
// Thread-safe: All operations protected by mutex
// Persistent: All writes logged to WAL
// Uses std::unordered_map for O(1) average-case operations
class Database {
public:
    Database();
    ~Database();
    
    // Enable WAL for persistence
    // Must be called before any operations
    bool enableWAL(const std::string& wal_filename);
    
    // Restore database state from WAL
    // Returns number of operations replayed
    size_t replayWAL(const std::string& wal_filename);
    
    // Store a key-value pair. If key exists, update it.
    // Returns true on success
    // Thread-safe, persistent (if WAL enabled)
    bool set(const std::string& key, const std::string& value);
    
    // Retrieve a value by key
    // Returns Optional with no value if key doesn't exist
    // Thread-safe
    Optional<std::string> get(const std::string& key) const;
    
    // Delete a key-value pair
    // Returns true if key existed, false otherwise
    // Thread-safe, persistent (if WAL enabled)
    bool del(const std::string& key);
    
    // Enable memory limit with LRU eviction
    // When limit is reached, least recently used keys are evicted
    // Size 0 = no limit
    void setMaxSize(size_t max_keys);
    
    // Set TTL for a key (expires after seconds)
    // Returns false if key doesn't exist
    bool expire(const std::string& key, int64_t seconds);
    
    // Get remaining TTL for a key
    // Returns -1 if no expiration, -2 if expired/not found
    int64_t ttl(const std::string& key) const;
    
    // Clean up expired keys
    // Returns number of keys deleted
    size_t cleanupExpired();
    
    // Check if a key exists
    // Thread-safe
    bool exists(const std::string& key) const;
    
    // Get the number of keys stored
    // Thread-safe
    size_t size() const;
    
    // Clear all data
    // Thread-safe
    void clear();

private:
    std::unordered_map<std::string, std::string> store_;
    mutable Mutex mutex_;  // mutable allows locking in const methods
    WAL* wal_;  // Write-ahead log (nullptr if disabled)
    
    // LRU and TTL management
    LRUCache* lru_;  // nullptr if no memory limit
    TTLManager ttl_manager_;
    size_t max_keys_;
    
    // Evict LRU key if at capacity
    // Must be called with mutex locked
    void evictIfNeeded();
};

} // namespace pebblekv
