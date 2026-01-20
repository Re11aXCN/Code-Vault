/*
 * @lc app=leetcode.cn id=217 lang=cpp
 *
 * [217] 存在重复元素
 */

// @lc code=start
class Solution {
public:
    bool containsDuplicate(vector<int>& nums) {
        std::unordered_set<int> filter; filter.reserve(nums.size());
        for (int num : nums) {
            // auto [it, success] = filter.emplace(num); !success
            if (!filter.emplace(num).second) return true;
        }
        return false;
    }
};
// @lc code=end

