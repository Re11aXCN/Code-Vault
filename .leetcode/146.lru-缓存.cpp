/*
 * @lc app=leetcode.cn id=146 lang=cpp
 *
 * [146] LRU 缓存
 */

// @lc code=start
class LRUCache {
public:
    LRUCache(int capacity) : capacity_(capacity) {}
    
    int get(int key) {
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return -1; // 未找到
        }
        
        // 移动到最近使用的位置
        items_list_.splice(items_list_.begin(), items_list_, it->second);
        return it->second->value;
    }
    
    void put(int key, int value) {
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            // 键已存在，更新值并移动到最近使用的位置
            it->second->value = value;
            items_list_.splice(items_list_.begin(), items_list_, it->second);
            return;
        }
        
        // 如果缓存已满，移除最久未使用的项
        if (items_list_.size() >= capacity_) {
            auto& last = items_list_.back();
            cache_map_.erase(last.key);
            items_list_.pop_back();
        }
        
        // 添加新项到最近使用的位置
        items_list_.push_front({key, value});
        cache_map_[key] = items_list_.begin();
    }

    // 检查键是否存在于缓存中
    bool contains(int key) const {
        return cache_map_.find(key) != cache_map_.end();
    }
    
    // 获取当前缓存大小
    size_t size() const {
        return items_list_.size();
    }
    
    // 获取缓存容量
    size_t capacity() const {
        return capacity_;
    }
    
    // 清空缓存
    void clear() {
        items_list_.clear();
        cache_map_.clear();
    }
private:
    struct Item {
        int key;
        int value;
    };
    
    size_t capacity_;
    std::list<Item> items_list_;
    std::unordered_map<int, typename std::list<Item>::iterator> cache_map_;

};
#include <list>
#include <optional>
#include <absl/container/flat_hash_map.h>

template <typename Key, typename Value>
class LRUCache {
public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {}
    
    // 从缓存中获取值，如果存在则移动到最近使用的位置
    std::optional<Value> get(const Key& key) {
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return std::nullopt; // 未找到
        }
        
        // 移动到最近使用的位置
        items_list_.splice(items_list_.begin(), items_list_, it->second);
        return it->second->value;
    }
    
    // 将键值对放入缓存
    void put(const Key& key, const Value& value) {
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            // 键已存在，更新值并移动到最近使用的位置
            it->second->value = value;
            items_list_.splice(items_list_.begin(), items_list_, it->second);
            return;
        }
        
        // 如果缓存已满，移除最久未使用的项
        if (items_list_.size() >= capacity_) {
            auto& last = items_list_.back();
            cache_map_.erase(last.key);
            items_list_.pop_back();
        }
        
        // 添加新项到最近使用的位置
        items_list_.push_front({key, value});
        cache_map_[key] = items_list_.begin();
    }
    
    // 检查键是否存在于缓存中
    bool contains(const Key& key) const {
        return cache_map_.find(key) != cache_map_.end();
    }
    
    // 获取当前缓存大小
    size_t size() const {
        return items_list_.size();
    }
    
    // 获取缓存容量
    size_t capacity() const {
        return capacity_;
    }
    
    // 清空缓存
    void clear() {
        items_list_.clear();
        cache_map_.clear();
    }

private:
    struct Item {
        Key key;
        Value value;
    };
    
    size_t capacity_;
    std::list<Item> items_list_;
    absl::flat_hash_map<Key, typename std::list<Item>::iterator> cache_map_;
};
#include <unordered_map>
#include <list>
#include <optional>

template <typename Key, typename Value>
class LFUCache {
private:
    struct Node {
        Key key;
        Value value;
        int frequency;
        Node(Key k, Value v, int freq) : key(k), value(v), frequency(freq) {}
    };

    // 频率到节点的映射，每个频率对应一个双向链表（最近访问的在链表头部）
    std::unordered_map<int, std::list<Node>> freq_map_;
    
    // 键到节点位置的映射
    std::unordered_map<Key, typename std::list<Node>::iterator> key_map_;
    
    size_t capacity_;
    int min_frequency_;  // 当前最小频率

public:
    explicit LFUCache(size_t capacity) : capacity_(capacity), min_frequency_(0) {}
    
    // 从缓存中获取值
    std::optional<Value> get(const Key& key) {
        if (capacity_ == 0) return std::nullopt;
        
        auto it = key_map_.find(key);
        if (it == key_map_.end()) {
            return std::nullopt;
        }
        
        // 获取节点迭代器
        auto node_it = it->second;
        Value value = node_it->value;
        int freq = node_it->frequency;
        
        // 从当前频率链表中移除
        freq_map_[freq].erase(node_it);
        
        // 如果当前频率链表为空且是最小频率，更新最小频率
        if (freq_map_[freq].empty()) {
            freq_map_.erase(freq);
            if (min_frequency_ == freq) {
                min_frequency_ = freq + 1;
            }
        }
        
        // 插入到更高频率的链表头部
        freq_map_[freq + 1].push_front(Node(key, value, freq + 1));
        key_map_[key] = freq_map_[freq + 1].begin();
        
        return value;
    }
    
    // 将键值对放入缓存
    void put(const Key& key, const Value& value) {
        if (capacity_ == 0) return;
        
        auto it = key_map_.find(key);
        if (it != key_map_.end()) {
            // 键已存在，更新值并增加频率
            auto node_it = it->second;
            int freq = node_it->frequency;
            
            // 从当前频率链表中移除
            freq_map_[freq].erase(node_it);
            
            // 如果当前频率链表为空且是最小频率，更新最小频率
            if (freq_map_[freq].empty()) {
                freq_map_.erase(freq);
                if (min_frequency_ == freq) {
                    min_frequency_ = freq + 1;
                }
            }
            
            // 插入到更高频率的链表头部
            freq_map_[freq + 1].push_front(Node(key, value, freq + 1));
            key_map_[key] = freq_map_[freq + 1].begin();
            return;
        }
        
        // 新键，需要检查容量
        if (key_map_.size() >= capacity_) {
            // 移除最小频率链表中的最后一个节点（最久未访问的）
            auto& min_freq_list = freq_map_[min_frequency_];
            Key key_to_remove = min_freq_list.back().key;
            min_freq_list.pop_back();
            key_map_.erase(key_to_remove);
            
            // 如果最小频率链表为空，清理
            if (min_freq_list.empty()) {
                freq_map_.erase(min_frequency_);
            }
        }
        
        // 插入新节点到频率1的链表头部
        min_frequency_ = 1;
        freq_map_[min_frequency_].push_front(Node(key, value, min_frequency_));
        key_map_[key] = freq_map_[min_frequency_].begin();
    }
    
    // 检查键是否存在于缓存中
    bool contains(const Key& key) const {
        return key_map_.find(key) != key_map_.end();
    }
    
    // 获取当前缓存大小
    size_t size() const {
        return key_map_.size();
    }
    
    // 获取缓存容量
    size_t capacity() const {
        return capacity_;
    }
    
    // 清空缓存
    void clear() {
        freq_map_.clear();
        key_map_.clear();
        min_frequency_ = 0;
    }
    
    // 获取键的当前访问频率（用于调试）
    std::optional<int> getFrequency(const Key& key) const {
        auto it = key_map_.find(key);
        if (it == key_map_.end()) {
            return std::nullopt;
        }
        return it->second->frequency;
    }
    
    // 获取当前最小频率（用于调试）
    int getMinFrequency() const {
        return min_frequency_;
    }
};
/**
 * Your LRUCache object will be instantiated and called as such:
 * LRUCache* obj = new LRUCache(capacity);
 * int param_1 = obj->get(key);
 * obj->put(key,value);
 */
// @lc code=end

/*
 * @lc app=leetcode.cn id=146 lang=cpp
 *
 * [146] LRU 缓存
 */

// @lc code=start
class LRUCache {
public:
    explicit LRUCache(int capacity) : capacity_(capacity) {}

    int get(int key) {
        auto it = cache_map_.find(key);
        if (it == cache_map_.end()) {
            return -1;
        }
        // 将访问的节点移动到链表头部
        cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
        return it->second->second;
    }

    void put(int key, int value) {
        auto it = cache_map_.find(key);
        if (it != cache_map_.end()) {
            // 更新已存在键的值，并移动到头部
            it->second->second = value;
            cache_list_.splice(cache_list_.begin(), cache_list_, it->second);
            return;
        }

        // 容量已满，删除LRU元素（链表尾部）
        if (cache_map_.size() >= capacity_) {
            auto& lru_node = cache_list_.back();
            cache_map_.erase(lru_node.first);
            cache_list_.pop_back();
        }

        // 插入新节点到链表头部，并更新哈希表
        cache_list_.emplace_front(key, value);
        cache_map_[key] = cache_list_.begin();
    }

private:
    int capacity_;
    std::list<std::pair<int, int>> cache_list_;
    std::unordered_map<int, std::list<std::pair<int, int>>::iterator> cache_map_;
};

/**
 * Your LRUCache object will be instantiated and called as such:
 * LRUCache* obj = new LRUCache(capacity);
 * int param_1 = obj->get(key);
 * obj->put(key,value);
 */
// @lc code=end

/*
 * LRUCache11 - a templated C++11 based LRU cache class that allows
 * specification of
 * key, value and optionally the map container type (defaults to
 * std::unordered_map)
 * By using the std::unordered_map and a linked list of keys it allows O(1) insert, delete
 * and
 * refresh operations.
 *
 * This is a header-only library and all you need is the LRUCache11.hpp file
 *
 * Github: https://github.com/mohaps/lrucache11
 *
 * This is a follow-up to the LRUCache project -
 * https://github.com/mohaps/lrucache
 *
 * Copyright (c) 2012-22 SAURAV MOHAPATRA <mohaps@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#pragma once
#include <algorithm>
#include <cstdint>
#include <list>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>

namespace lru11 {
    /*
     * a noop lockable concept that can be used in place of std::mutex
     */
    class NullLock {
    public:
        void lock() {}
        void unlock() {}
        bool try_lock() { return true; }
    };

    /**
     * error raised when a key not in cache is passed to get()
     */
    class KeyNotFound : public std::invalid_argument {
    public:
        KeyNotFound() : std::invalid_argument("key_not_found") {}
    };

    template <typename K, typename V>
    struct KeyValuePair {
    public:
        K key;
        V value;

        KeyValuePair(K k, V v) : key(std::move(k)), value(std::move(v)) {}
    };

    /**
     *	The LRU LRUCache class templated by
     *		Key - key type
     *		Value - value type
     *		MapType - an associative container like std::unordered_map
     *		LockType - a lock type derived from the Lock class (default:
     *NullLock = no synchronization)
     *
     *	The default NullLock based template is not thread-safe, however passing
     *Lock=std::mutex will make it
     *	thread-safe
     */
    template <class Key, class Value, class Lock = NullLock,
        class Map = std::unordered_map<
        Key, typename std::list<KeyValuePair<Key, Value>>::iterator>>
        class LRUCache {
        public:
            typedef KeyValuePair<Key, Value> node_type;
            typedef std::list<KeyValuePair<Key, Value>> list_type;
            typedef Map map_type;
            typedef Lock lock_type;
            using Guard = std::lock_guard<lock_type>;
            /**
             * the maxSize is the soft limit of keys and (maxSize + elasticity) is the
             * hard limit
             * the cache is allowed to grow till (maxSize + elasticity) and is pruned back
             * to maxSize keys
             * set maxSize = 0 for an unbounded cache (but in that case, you're better off
             * using a std::unordered_map
             * directly anyway! :)
             */
            explicit LRUCache(size_t maxSize = 64, size_t elasticity = 10)
                : maxSize_(maxSize), elasticity_(elasticity) {
            }
            virtual ~LRUCache() = default;
            size_t size() const {
                Guard g(lock_);
                return cache_.size();
            }
            bool empty() const {
                Guard g(lock_);
                return cache_.empty();
            }
            void clear() {
                Guard g(lock_);
                cache_.clear();
                keys_.clear();
            }
            void insert(const Key& k, Value v) {
                Guard g(lock_);
                const auto iter = cache_.find(k);
                if (iter != cache_.end()) {
                    iter->second->value = v;
                    keys_.splice(keys_.begin(), keys_, iter->second);
                    return;
                }

                keys_.emplace_front(k, std::move(v));
                cache_[k] = keys_.begin();
                prune();
            }
            void emplace(const Key& k, Value&& v) {
                Guard g(lock_);
                keys_.emplace_front(k, std::move(v));
                cache_[k] = keys_.begin();
                prune();
            }
            /**
              for backward compatibity. redirects to tryGetCopy()
             */
            bool tryGet(const Key& kIn, Value& vOut) {
                return tryGetCopy(kIn, vOut);
            }

            bool tryGetCopy(const Key& kIn, Value& vOut) {
                Guard g(lock_);
                Value tmp;
                if (!tryGetRef_nolock(kIn, tmp)) { return false; }
                vOut = tmp;
                return true;
            }

            bool tryGetRef(const Key& kIn, Value& vOut) {
                Guard g(lock_);
                return tryGetRef_nolock(kIn, vOut);
            }
            /**
             *	The const reference returned here is only
             *    guaranteed to be valid till the next insert/delete
             *  in multi-threaded apps use getCopy() to be threadsafe
             */
            const Value& getRef(const Key& k) {
                Guard g(lock_);
                return get_nolock(k);
            }

            /**
                added for backward compatibility
             */
            Value get(const Key& k) {
                return getCopy(k);
            }
            /**
             * returns a copy of the stored object (if found)
             * safe to use/recommended in multi-threaded apps
             */
            Value getCopy(const Key& k) {
                Guard g(lock_);
                return get_nolock(k);
            }

            bool remove(const Key& k) {
                Guard g(lock_);
                auto iter = cache_.find(k);
                if (iter == cache_.end()) {
                    return false;
                }
                keys_.erase(iter->second);
                cache_.erase(iter);
                return true;
            }
            bool contains(const Key& k) const {
                Guard g(lock_);
                return cache_.find(k) != cache_.end();
            }

            size_t getMaxSize() const { return maxSize_; }
            size_t getElasticity() const { return elasticity_; }
            size_t getMaxAllowedSize() const { return maxSize_ + elasticity_; }
            template <typename F>
            void cwalk(F& f) const {
                Guard g(lock_);
                std::for_each(keys_.begin(), keys_.end(), f);
            }

        protected:
            const Value& get_nolock(const Key& k) {
                const auto iter = cache_.find(k);
                if (iter == cache_.end()) {
                    throw KeyNotFound();
                }
                keys_.splice(keys_.begin(), keys_, iter->second);
                return iter->second->value;
            }
            bool tryGetRef_nolock(const Key& kIn, Value& vOut) {
                const auto iter = cache_.find(kIn);
                if (iter == cache_.end()) {
                    return false;
                }
                keys_.splice(keys_.begin(), keys_, iter->second);
                vOut = iter->second->value;
                return true;
            }
            size_t prune() {
                size_t maxAllowed = maxSize_ + elasticity_;
                if (maxSize_ == 0 || cache_.size() < maxAllowed) {
                    return 0;
                }
                size_t count = 0;
                while (cache_.size() > maxSize_) {
                    cache_.erase(keys_.back().key);
                    keys_.pop_back();
                    ++count;
                }
                return count;
            }

        private:
            // Disallow copying.
            LRUCache(const LRUCache&) = delete;
            LRUCache& operator=(const LRUCache&) = delete;

            mutable Lock lock_;
            Map cache_;
            list_type keys_;
            size_t maxSize_;
            size_t elasticity_;
    };

}  // namespace LRUCache11
