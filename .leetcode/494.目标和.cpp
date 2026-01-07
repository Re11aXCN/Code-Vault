#include "header.h"
/*
 * @lc app=leetcode.cn id=494 lang=cpp
 *
 * [494] 目标和
 */

// @lc code=start
class Solution {
public:
    // 拆分为正数、负数两个集合，集合相加等于target的个数，转换为0-1背包问题
    int findTargetSumWays(std::vector<int>& nums, int target) {
        int totalSum{0};
        #pragma GCC unroll 8
        for (int num : nums) totalSum += num;
        // int totalSum = std::accumulate(nums.begin(), nums.end(), 0);
        if (std::abs(target) > totalSum) return 0;

        int positiveSize = totalSum + target;
        /*
        考虑这样的情况，nums的和为奇数，那么不能拆分为一个正数和为偶数的集合和一个负数和为偶数的集合
        只能是一奇一偶，对于偶数和奇数的加减法，不可能得到偶数，所以如果nums和为奇数，且target的值为偶数，则不会满足条件
        */
        // 奇数不能被整除 说明无法满足
        if (positiveSize & 1) return 0;
        positiveSize >>= 1;

        std::vector<int> dp(positiveSize + 1, 0);
        dp[0] = 1; //满足递推公式的推到，如果都是0那么不论如何累加都是0，不对，空集也是一种？
        for(int num : nums)
        {
            #pragma GCC unroll 4
            for(int j = positiveSize; j >= num; --j)
            {
                dp[j] += dp[j - num];   // 多少种方式固定写法
            }
        }
        return dp[positiveSize];
    }
};
// @lc code=end

