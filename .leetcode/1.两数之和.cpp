/*
 * @lc app=leetcode.cn id=1 lang=cpp
 *
 * [1] 两数之和
 */

// @lc code=start
class Solution {
public:

    vector<int> twoSum(vector<int>& nums, int target) {
        std::unordered_map<int, int> complements;
        int i = 0;
        for (vector<int>::iterator it = nums.begin(); it != nums.end(); ++it, ++i) {
            int diff = target - *it;

            if (auto mit = complements.find(diff); mit != complements.end()) {
                return { i, mit->second };
            }
            complements.emplace(*it, i);
        }
        return {};
    }
};
// @lc code=end

