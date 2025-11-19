#include "header.h"
/*
 * @lc app=leetcode.cn id=1049 lang=cpp
 *
 * [1049] 最后一块石头的重量 II
 */

// @lc code=start
class Solution {
public:
    int lastStoneWeightII(std::vector<int>& stones) {
        std::size_t totalSum{0};

        #pragma GCC unroll 8
        for(int num : stones) totalSum += num;

        std::size_t half{ totalSum >> 1 };
        std::vector<std::size_t> dp(half + 1, 0);
        
        for(int num : stones) {
            #pragma GCC unroll 4
            for(std::size_t j = half; j >= num; --j) {
                if(int value = dp[j - num] + num; value > dp[j])
                    dp[j] = value;
            }
        }
        return totalSum - (dp.back() << 1);
    }
};
// @lc code=end

