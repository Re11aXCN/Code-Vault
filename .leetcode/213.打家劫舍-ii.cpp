/*
 * @lc app=leetcode.cn id=213 lang=cpp
 *
 * [213] 打家劫舍 II
 */

// @lc code=start
class Solution {
public:
    int rob(vector<int>& nums) {
        int rob = nums[0], not_rob = 0, maxMoney = rob;

        for (int i = 1; i < nums.size() - 1; ++i) {
            if(int currMoney = not_rob + nums[i]; currMoney > maxMoney)
                maxMoney = currMoney;
            not_rob = rob;
            rob = maxMoney;
        }

        
        return maxMoney;
    }
};
// @lc code=end

