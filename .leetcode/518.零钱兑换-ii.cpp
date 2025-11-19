#include "header.h"
/*
 * @lc app=leetcode.cn id=518 lang=cpp
 *
 * [518] 零钱兑换 II
 */

// @lc code=start
class Solution {
public:
    int change(int amount, std::vector<int>& coins) {
        std::vector<std::size_t> dp(amount + 1, 0);
        dp[0] = 1;
        for(int num : coins) {
            #pragma clang loop vectorize(enable) unroll_count(8)
            for(int j = num; j <= amount; ++j) {
                dp[j] += dp[j - num];
            }
        }
        return dp.back();
    }
};
// @lc code=end

