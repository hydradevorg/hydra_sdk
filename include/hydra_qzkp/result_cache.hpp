#pragma once
#include <unordered_map>
#include <deque>
#include <mutex>
#include <optional>
#include <chrono>
#include <string>

namespace hydra::qzkp {

template <typename Key, typename Value>
class ResultCache {
public:
    explicit ResultCache(size_t maxsize = 10000);

    std::optional<Value> get(const Key& key);
    void put(const Key& key, const Value& value);

private:
    void evict_oldest();

    std::unordered_map<Key, Value> cache_;
    std::unordered_map<Key, double> access_time_;
    std::deque<Key> access_queue_;
    std::mutex mutex_;
    size_t maxsize_;
};

} // namespace hydra::qzkp
