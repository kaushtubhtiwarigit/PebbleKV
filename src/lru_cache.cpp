#include "lru_cache.h"
#include <chrono>
#include <algorithm>

namespace pebblekv {

// LRUCache implementation

LRUCache::LRUCache(size_t max_size) : max_size_(max_size) {}

void LRUCache::touch(const std::string& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
        // Move to front (most recently used)
        items_.splice(items_.begin(), items_, it->second);
    } else {
        add(key);
    }
}

void LRUCache::add(const std::string& key) {
    // Add to front of list
    items_.push_front(key);
    map_[key] = items_.begin();
}

void LRUCache::remove(const std::string& key) {
    auto it = map_.find(key);
    if (it != map_.end()) {
        items_.erase(it->second);
        map_.erase(it);
    }
}

std::string LRUCache::getLRU() {
    if (items_.empty()) {
        return "";
    }
    return items_.back();  // Back = least recently used
}

// TTLManager implementation

void TTLManager::setExpiration(const std::string& key, int64_t seconds) {
    int64_t expiration = currentTime() + seconds;
    expirations_[key] = expiration;
}

void TTLManager::removeExpiration(const std::string& key) {
    expirations_.erase(key);
}

bool TTLManager::isExpired(const std::string& key) const {
    auto it = expirations_.find(key);
    if (it == expirations_.end()) {
        return false;  // No expiration set
    }
    return currentTime() >= it->second;
}

int64_t TTLManager::getTTL(const std::string& key) const {
    auto it = expirations_.find(key);
    if (it == expirations_.end()) {
        return -1;  // No expiration
    }
    
    int64_t remaining = it->second - currentTime();
    if (remaining < 0) {
        return -2;  // Expired
    }
    return remaining;
}

std::vector<std::string> TTLManager::getExpiredKeys() const {
    std::vector<std::string> expired;
    int64_t now = currentTime();
    
    for (const auto& pair : expirations_) {
        if (now >= pair.second) {
            expired.push_back(pair.first);
        }
    }
    
    return expired;
}

int64_t TTLManager::currentTime() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

} // namespace pebblekv
