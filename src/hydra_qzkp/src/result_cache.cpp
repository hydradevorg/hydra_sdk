#include "hydra_qzkp/result_cache.hpp"

namespace hydra::qzkp {

template <typename Key, typename Value>
ResultCache<Key, Value>::ResultCache(size_t maxsize)
    : maxsize_(maxsize) {}

template <typename Key, typename Value>
std::optional<Value> ResultCache<Key, Value>::get(const Key& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        access_time_[key] = std::chrono::duration<double>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        access_queue_.push_back(key);
        return it->second;
    }
    return std::nullopt;
}

template <typename Key, typename Value>
void ResultCache<Key, Value>::put(const Key& key, const Value& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (cache_.size() >= maxsize_) {
        evict_oldest();
    }
    cache_[key] = value;
    access_time_[key] = std::chrono::duration<double>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    access_queue_.push_back(key);
}

template <typename Key, typename Value>
void ResultCache<Key, Value>::evict_oldest() {
    while (!access_queue_.empty() && cache_.size() >= maxsize_) {
        Key oldest = access_queue_.front();
        access_queue_.pop_front();
        if (cache_.contains(oldest)) {
            cache_.erase(oldest);
            access_time_.erase(oldest);
        }
    }
}

// Explicit instantiation for std::string → bool or string → any
template class ResultCache<std::string, bool>;
template class ResultCache<std::string, std::string>;

} // namespace hydra::qzkp
