#include "kv_store.h"
#include <fstream>
#include <sstream>

using namespace std::chrono;

bool KVStore::is_expired(const Entry& e) const {
    if (!e.has_expiry) return false;
    return steady_clock::now() > e.expiry;
}

void KVStore::set(const std::string& key, const std::string& value) {
    std::unique_lock lock(mutex_);
    data_[key] = Entry{value, {}, false};
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::unique_lock lock(mutex_);  // upgrade to unique to allow lazy delete
    auto it = data_.find(key);
    if (it == data_.end()) return std::nullopt;
    if (is_expired(it->second)) {
        data_.erase(it);
        return std::nullopt;
    }
    return it->second.value;
}

bool KVStore::del(const std::string& key) {
    std::unique_lock lock(mutex_);
    return data_.erase(key) > 0;
}

bool KVStore::expire(const std::string& key, int seconds) {
    std::unique_lock lock(mutex_);
    auto it = data_.find(key);
    if (it == data_.end()) return false;
    it->second.has_expiry = true;
    it->second.expiry = steady_clock::now() + seconds * 1s;
    return true;
}

long long KVStore::ttl(const std::string& key) {
    std::shared_lock lock(mutex_);
    auto it = data_.find(key);
    if (it == data_.end()) return -2;
    if (is_expired(it->second)) return -2;
    if (!it->second.has_expiry) return -1;
    auto diff = duration_cast<seconds>(it->second.expiry - steady_clock::now()).count();
    return diff < 0 ? -2 : diff;
}

void KVStore::save_snapshot(const std::string& path) {
    std::shared_lock lock(mutex_);
    std::ofstream out(path, std::ios::trunc);
    for (const auto& [k, v] : data_) {
        if (is_expired(v)) continue;
        out << k.size() << '\n' << k << '\n'
            << v.value.size() << '\n' << v.value << '\n'
            << (v.has_expiry ? 1 : 0) << '\n';
        if (v.has_expiry) {
            auto remaining = duration_cast<seconds>(v.expiry - steady_clock::now()).count();
            out << remaining << '\n';
        }
    }
}

void KVStore::load_snapshot(const std::string& path) {
    std::unique_lock lock(mutex_);
    std::ifstream in(path);
    if (!in) return;
    std::string line;
    while (std::getline(in, line)) {
        size_t klen = std::stoul(line);
        std::string key(klen, '\0');
        in.read(&key[0], klen); in.get();  // newline
        std::getline(in, line);
        size_t vlen = std::stoul(line);
        std::string val(vlen, '\0');
        in.read(&val[0], vlen); in.get();
        std::getline(in, line);
        bool has_exp = line == "1";
        Entry e{val, {}, has_exp};
        if (has_exp) {
            std::getline(in, line);
            int secs = std::stoi(line);
            e.expiry = steady_clock::now() + secs * 1s;
        }
        data_[key] = std::move(e);
    }
}