#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <optional>

class KVStore {
public:
    // Basic ops
    void set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    
    // TTL ops
    bool expire(const std::string& key, int seconds);
    long long ttl(const std::string& key);  // -1 = no expiry, -2 = not found
    
    // Persistence
    void save_snapshot(const std::string& path);
    void load_snapshot(const std::string& path);

private:
    struct Entry {
        std::string value;
        std::chrono::steady_clock::time_point expiry;
        bool has_expiry = false;
    };
    
    bool is_expired(const Entry& e) const;
    
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, Entry> data_;
};