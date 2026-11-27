#include "database.h"
#include "wal.h"
#include <iostream>

namespace pebblekv {

Database::Database() : wal_(nullptr), lru_(nullptr), max_keys_(0) {}

Database::~Database() {
    if (wal_) {
        delete wal_;
    }
    if (lru_) {
        delete lru_;
    }
}

bool Database::enableWAL(const std::string& wal_filename) {
    LockGuard lock(mutex_);
    
    if (wal_) {
        std::cerr << "WAL already enabled\n";
        return false;
    }
    
    wal_ = new WAL(wal_filename);
    if (!wal_->open()) {
        delete wal_;
        wal_ = nullptr;
        return false;
    }
    
    std::cout << "WAL enabled: " << wal_filename << "\n";
    return true;
}

size_t Database::replayWAL(const std::string& wal_filename) {
    if (!WALReader::exists(wal_filename)) {
        return 0;  // No WAL file, fresh start
    }
    
    std::cout << "Replaying WAL: " << wal_filename << "\n";
    
    WALReader reader(wal_filename);
    if (!reader.open()) {
        std::cerr << "Failed to open WAL for replay\n";
        return 0;
    }
    
    size_t count = 0;
    std::string command, key, value;
    
    while (reader.readNext(command, key, value)) {
        if (command == "SET") {
            LockGuard lock(mutex_);
            store_[key] = value;
            count++;
        } else if (command == "DEL") {
            LockGuard lock(mutex_);
            store_.erase(key);
            count++;
        }
    }
    
    reader.close();
    std::cout << "Replayed " << count << " operations from WAL\n";
    
    return count;
}

void Database::setMaxSize(size_t max_keys) {
    LockGuard lock(mutex_);
    max_keys_ = max_keys;
    
    if (max_keys > 0) {
        if (!lru_) {
            lru_ = new LRUCache(max_keys);
            // Add existing keys to LRU
            for (const auto& pair : store_) {
                lru_->add(pair.first);
            }
        }
        std::cout << "Memory limit enabled: " << max_keys << " keys max\n";
    } else {
        if (lru_) {
            delete lru_;
            lru_ = nullptr;
        }
        std::cout << "Memory limit disabled\n";
    }
}

bool Database::expire(const std::string& key, int64_t seconds) {
    LockGuard lock(mutex_);
    
    if (store_.find(key) == store_.end()) {
        return false;  // Key doesn't exist
    }
    
    ttl_manager_.setExpiration(key, seconds);
    return true;
}

int64_t Database::ttl(const std::string& key) const {
    LockGuard lock(mutex_);
    return ttl_manager_.getTTL(key);
}

size_t Database::cleanupExpired() {
    std::vector<std::string> expired = ttl_manager_.getExpiredKeys();
    
    for (const std::string& key : expired) {
        del(key);  // Will acquire lock internally
    }
    
    return expired.size();
}

void Database::evictIfNeeded() {
    // Must be called with mutex locked
    if (!lru_ || !lru_->isFull()) {
        return;
    }
    
    // Evict LRU key
    std::string lru_key = lru_->getLRU();
    if (!lru_key.empty()) {
        std::cout << "Evicting LRU key: " << lru_key << "\n";
        store_.erase(lru_key);
        lru_->remove(lru_key);
        ttl_manager_.removeExpiration(lru_key);
        
        // Log eviction to WAL as DELETE
        if (wal_) {
            wal_->logDelete(lru_key);
        }
    }
}

bool Database::set(const std::string& key, const std::string& value) {
    LockGuard lock(mutex_);
    
    // Clean up expired first
    if (ttl_manager_.isExpired(key)) {
        store_.erase(key);
        ttl_manager_.removeExpiration(key);
        if (lru_) {
            lru_->remove(key);
        }
    }
    
    // Check if we need to evict
    bool is_new_key = (store_.find(key) == store_.end());
    if (is_new_key) {
        evictIfNeeded();
    }
    
    // Log to WAL first (write-ahead)
    if (wal_) {
        if (!wal_->logSet(key, value)) {
            std::cerr << "Failed to write to WAL\n";
            return false;
        }
    }
    
    // Update in-memory store
    store_[key] = value;
    
    // Update LRU
    if (lru_) {
        if (is_new_key) {
            lru_->add(key);
        } else {
            lru_->touch(key);
        }
    }
    
    return true;
}

Optional<std::string> Database::get(const std::string& key) const {
    LockGuard lock(mutex_);
    
    // Check expiration
    if (ttl_manager_.isExpired(key)) {
        // Key expired, treat as not found
        // Note: we can't delete here (const method)
        return Optional<std::string>();
    }
    
    auto it = store_.find(key);
    if (it != store_.end()) {
        // Update LRU (const_cast needed for mutable LRU in const method)
        if (lru_) {
            const_cast<LRUCache*>(lru_)->touch(key);
        }
        return Optional<std::string>(it->second);
    }
    return Optional<std::string>();
}

bool Database::del(const std::string& key) {
    LockGuard lock(mutex_);
    
    // Log to WAL first
    if (wal_) {
        if (!wal_->logDelete(key)) {
            std::cerr << "Failed to write to WAL\n";
            return false;
        }
    }
    
    // Delete from in-memory store
    bool deleted = store_.erase(key) > 0;
    
    if (deleted) {
        // Remove from LRU and TTL
        if (lru_) {
            lru_->remove(key);
        }
        ttl_manager_.removeExpiration(key);
    }
    
    return deleted;
}

bool Database::exists(const std::string& key) const {
    LockGuard lock(mutex_);
    return store_.find(key) != store_.end();
}

size_t Database::size() const {
    LockGuard lock(mutex_);
    return store_.size();
}

void Database::clear() {
    LockGuard lock(mutex_);
    store_.clear();
}

} // namespace pebblekv
