#include "header.h"
/*
 * @lc app=leetcode.cn id=474 lang=cpp
 *
 * [474] 一和零
 */
// https://quick-bench.com/q/jwXnXqVYzNuSMYYkQ_bgf_qV-34
// @lc code=start

class Solution {
public:
    int findMaxForm(std::vector<std::string>& strs, int m, int n) {
        std::vector<int> dp((m + 1) * (n + 1), 0);
        // 0-1 背包 变种 一个物品装在两个背包中，满足价值最大
        #pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
        for (auto& str : strs) { // 一个物品
            int zeroCount = 0, oneCount = 0;
            #pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
            for (char c : str) {
                c == '0' ? ++zeroCount : ++oneCount;
            }
            int offset = zeroCount * (n + 1) + oneCount;
            
            #pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
            for (int i = m; i >= zeroCount; --i) { // 两个背包
                #pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
                for (int j = n; j >= oneCount; --j) {
                    int idx = i * (n + 1) + j;
                    // int prev_idx = (i - zeroCount) * (n + 1) + (j - oneCount);
                    int pre_idx = idx - offset;
                    dp[idx] = std::max(dp[idx], dp[prev_idx] + 1);
                }
            }
        }
        return dp.back(); // dp[m * (n + 1) + n];
    }
};
// @lc code=end
class Solution {
public:
    int findMaxForm(std::vector<std::string>& strs, int m, int n) {
        std::vector<std::vector<int>> dp(m + 1, std::vector<int> (n + 1, 0));
        // 0-1 背包 变种 一个物品装在两个背包中，满足价值最大
        #pragma GCC unroll 8
        for (auto& str : strs) { // 一个物品
            int oneCount = 0, zeroCount = 0;
            #pragma GCC unroll 8
            for (char c : str) {
                c == '0' ? ++zeroCount : ++oneCount;
            }
            #pragma GCC unroll 8
            for (int i = m; i >= zeroCount; --i) { // 两个背包
                for (int j = n; j >= oneCount; --j) {
                    dp[i][j] = std::max(dp[i][j], dp[i - zeroCount][j - oneCount] + 1);
                }
            }
        }
        return dp[m][n];
    }
};

