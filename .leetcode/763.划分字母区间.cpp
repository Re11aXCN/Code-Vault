// 最优解，原地hash映射，每个字符的最远位置，第二次遍历计算分割范围通过right、left
// 代码随想录
class Solution {
public:
    std::vector<int> partitionLabels(std::string s) {
        static std::array<int, 26> alpha;
        #pragma GCC unroll 4
        for(int i = 0; i < s.size(); ++i)  alpha[s[i] - 'a'] = i;

        std::vector<int> result;
        int left{0}, right{0};
        #pragma GCC unroll 4
        for(int i = 0; i < s.size(); ++i) {
            right = std::max(right, alpha[s[i] - 'a']);

            if(i == right) {
                result.push_back(right - left + 1);
                left = i + 1;
            }
        }
        return result;
    }
};
/*
 * @lc app=leetcode.cn id=763 lang=cpp
 *
 * [763] 划分字母区间
 */

// @lc code=start
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>
#include <ranges>
#include <algorithm>
using namespace std;
class Solution {
    template <typename Key, typename Value>
    class LinkedHashMap {
private:
    std::vector<Key> order; // 维护插入顺序
    std::unordered_map<Key, std::pair<Value, size_t>> data; // Key → (Value, order索引)

public:
    // 插入或更新键值对（更新时保留原插入顺序）
    void insert(const Key& key, const Value& val) {
        if (!contains(key)) {
            order.push_back(key);
            data[key] = {val, order.size() - 1}; // 新键：记录值和索引
        } else {
            data[key].first = val; // 已存在：仅更新值，保持原索引
        }
    }

    // 获取值（非const版本）
    Value& get(const Key& key) {
        if (!contains(key)) {
            throw std::out_of_range("Key not found");
        }
        return data[key].first;
    }

    // 获取值（const版本）
    const Value& get(const Key& key) const {
        if (!contains(key)) {
            throw std::out_of_range("Key not found");
        }
        return data.at(key).first;
    }

    // 检查键是否存在
    bool contains(const Key& key) const {
        return data.find(key) != data.end();
    }

    // 返回元素数量
    size_t size() const {
        return order.size();
    }

    // 迭代器支持（按插入顺序遍历键）
    typename std::vector<Key>::iterator begin() { return order.begin(); }
    typename std::vector<Key>::iterator end() { return order.end(); }
    typename std::vector<Key>::const_iterator begin() const { return order.begin(); }
    typename std::vector<Key>::const_iterator end() const { return order.end(); }
    };
public:
    vector<int> partitionLabels(string s) {
//#define LINKED_HASH_MAP
#ifdef LINKED_HASH_MAP
    LinkedHashMap<char, pair<int, int>> filter;

    for (int i = 0; i < s.size(); ++i) {
        if (filter.contains(s[i])) {
            filter.get(s[i]).second = i;
        } else {
            filter.insert(s[i], {i, i});
        }
    }

    vector<pair<int, int>> vec;
    for (char c : filter) { // 按插入顺序遍历
        vec.push_back(filter.get(c));
    }

    return mergeAndCalculate(vec);  
#else
        unordered_map<char, pair<int,int>> filter;
        filter.reserve(26);
        for(int i = 0; i < s.length(); ++i){
            if(filter.contains(s[i])){
                filter[s[i]].second = i;
            }
            else{
                filter[s[i]] = {i, i};
            }
        }
        auto vec = filter | views::transform([](const auto& kv) {
            return kv.second;
            }) | ranges::to<vector<pair<int, int>>>();
        ranges::sort(vec, [](const auto& a, const auto& b) {
            return a.first < b.first;
            });
#endif
        //计算长度
        return mergeAndCalculate(vec);
    }
    vector<int> mergeAndCalculate(const vector<pair<int, int>>& intervals) {
        vector<int> result;
        if (intervals.empty()) return result;

        vector<pair<int, int>> merged;
        merged.push_back(intervals[0]);

        for (size_t i = 1; i < intervals.size(); ++i) {
            pair<int, int> current = intervals[i];
            pair<int, int>& last = merged.back();

            if (current.first <= last.second) {
                // 合并区间
                last.second = max(last.second, current.second);
            }
            else {
                merged.push_back(current);
            }
        }

        for (const auto& interval : merged) {
            result.push_back(interval.second - interval.first + 1);
        }

        return result;
    }
};
// @lc code=end
int main(){
    Solution s;
    string str = "ababcbacadefegdehijhklij";
    auto res = s.partitionLabels(str);
    return 0;
}

