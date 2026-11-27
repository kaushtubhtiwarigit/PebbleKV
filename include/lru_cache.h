#pragma once

#include <string>
#include <unordered_map>
#include <list>
#include <cstdint>

namespace pebblekv {

// Simple LRU (Least Recently Used) cache for key eviction
// When cache is full, removes least recently accessed keys
class LRUCache {
public:
    explicit LRUCache(size_t max_size);
    
    // Mark key as accessed (moves to front)
    void touch(const std::string& key);
    
    // Add key to cache
    void add(const std::string& key);
    
    // Remove key from cache
    void remove(const std::string& key);
    
    // Get least recently used key (for eviction)
    // Returns empty string if cache is empty
    std::string getLRU();
    
    // Check if cache is at capacity
    bool isFull() const { return items_.size() >= max_size_; }
    
    // Get current size
    size_t size() const { return items_.size(); }
    
    // Get max size
    size_t maxSize() const { return max_size_; }

private:
    size_t max_size_;
    
    // List maintains access order (front = most recent)
    std::list<std::string> items_;
    
    // Map for O(1) lookup of list iterators
    std::unordered_map<std::string, std::list<std::string>::iterator> map_;
};

// Time-to-Live (TTL) manager
// Tracks expiration times for keys
class TTLManager {
public:
    // Set expiration for a key (seconds from now)
    void setExpiration(const std::string& key, int64_t seconds);
    
    // Remove expiration for a key
    void removeExpiration(const std::string& key);
    
    // Check if key has expired
    bool isExpired(const std::string& key) const;
    
    // Get TTL for a key (seconds remaining, -1 if no expiration, -2 if expired/not found)
    int64_t getTTL(const std::string& key) const;
    
    // Get all expired keys (for cleanup)
    std::vector<std::string> getExpiredKeys() const;
    
    // Get current timestamp (seconds since epoch)
    static int64_t currentTime();

private:
    // Map key → expiration timestamp
    std::unordered_map<std::string, int64_t> expirations_;
};

} // namespace pebblekv
