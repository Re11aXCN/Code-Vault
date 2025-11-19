// 桶排序思想
class Solution {
public:
    std::vector<int> topKFrequent(std::vector<int>& nums, int k) {
        // 1. 统计频率
        std::unordered_map<int, int> freqNumMap;

        #pragma GCC unroll 4
        for(int num : nums) freqNumMap[num]++;

        // 2. 桶排序 - 以频率为索引
        // 最大频率不会超过数组长度
        std::vector<int> sameFreVec; sameFreVec.reserve(8);
        std::vector<std::vector<int>> buckets(nums.size() + 1, sameFreVec);
        for (auto& [num, count] : freqNumMap) {
            buckets[count].push_back(num);
        }

        std::vector<int> result;
        result.reserve(k);

        // 3. 收集结果 - 从高频到低频
        #pragma GCC unroll 4
        for(int freq = nums.size(); freq >= 0 && result.size() < k; --freq)
        {
            for(int num : buckets[freq]) {
                result.push_back(num);
                if (result.size() == k) break;
            }
        }
        return result;
    }
};

/*
 * @lc app=leetcode.cn id=347 lang=cpp
 *
 * [347] 前 K 个高频元素
 */

// @lc code=start
#include <unordered_map>
#include <queue>
using namespace std;
class Solution {
    struct CompareByValue {
        bool operator()(const pair<int, int>& a, const pair<int, int>& b) {
            return a.second < b.second; // 最大堆逻辑（top 为最大元素）
        }
    };
public:
    vector<int> topKFrequent(vector<int>& nums, int k) {
        unordered_map<int, int> num_count;
        for(int n : nums) num_count[n] = num_count.contains(n) ? num_count[n] + 1 : 1;
        // 定义优先队列：元素为 pair<key,value>，容器为 vector，比较器为 CompareByValue
        priority_queue<
            pair<int, int>,
            vector<pair<int, int>>,
            CompareByValue
        > max_heap {num_count.begin(), num_count.end()};
        vector<int> result;
        result.reserve(k);
        while (k != 0) {
            auto[value, count] = max_heap.top();
            result.push_back(value);
            max_heap.pop();
            --k;
        }
        return result;
    }
};
// @lc code=end

