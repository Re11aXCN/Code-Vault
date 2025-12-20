#include "header.h"
/*
 * @lc app=leetcode.cn id=1049 lang=cpp
 *
 * [1049] 最后一块石头的重量 II
 */

// @lc code=start
class Solution {
public:
    // 拆分为两个集合，两个集合的差值尽可能小，两个集合相撞（相减）就是结果
    // 转换为 0-1背包问题，求其中一半集合的最大重量，最后结果减掉这个集合的最大总量的一半就是结果
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

