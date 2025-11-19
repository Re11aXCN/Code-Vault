#include "header.h"
/*
 * @lc app=leetcode.cn id=494 lang=cpp
 *
 * [494] 目标和
 */

// @lc code=start
class Solution {
public:
    int findTargetSumWays(std::vector<int>& nums, int target) {
        int sum{0};
        #pragma GCC unroll 8
        for (int num : nums) sum += num;

        if (std::abs(target) > sum) return 0;

        int positiveSize = sum + target;
        // 奇数不能被整除 说明无法满足
        if (positiveSize & 1) return 0;
        positiveSize >>= 1;

        std::vector<int> dp(positiveSize + 1, 0);
        dp[0] = 1;
        for(int num : nums)
        {
            #pragma GCC unroll 4
            for(int j = positiveSize; j >= num; --j)
            {
                dp[j] += dp[j - num];
            }
        }
        return dp[positiveSize];
    }
};
// @lc code=end

