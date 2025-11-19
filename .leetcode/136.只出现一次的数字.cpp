/*
 * @lc app=leetcode.cn id=136 lang=cpp
 *
 * [136] 只出现一次的数字
 */

// @lc code=start
class Solution {
public:
    int singleNumber(vector<int>& nums) {
        int result = 0;
        #pragma GCC unroll 8
        for (int i = 0; i < nums.size(); ++i) {
            result = nums[i] ^ result;
        }
        return result;
    }
};
// @lc code=end

