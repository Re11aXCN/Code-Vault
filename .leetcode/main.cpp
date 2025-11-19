#include <stack>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <numeric>
#include <ranges>
#include <queue>
#include <cstdint>
#include <cstddef>
#include <string>
#include <climits>
#include <array>
#include <charconv>
#include <execution>
#include <immintrin.h>
/*
 * @lc app=leetcode.cn id=322 lang=cpp
 *
 * [322] 零钱兑换
 */

// @lc code=start
class Solution {
public:
    int coinChange(vector<int>& coins, int amount) {
        if (amount == 0) return 0;
        
        std::vector<std::size_t> dp(amount + 1, amount + 1);
        dp[0] = 0;
        #pragma clang loop interleave(enable) unroll_count(8)
        for (int coin : coins) {
            #pragma clang loop interleave(enable) unroll_count(8)
            for (int j = coin; j <= amount; ++j) {
                if(auto value = dp[j - coin] + 1; value < dp[j]) dp[j] = value;
            }
        }
        
        return dp[amount] > amount ? -1 : dp[amount];
    }
};
// @lc code=end