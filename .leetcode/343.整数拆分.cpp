#include <cstdlib>
#include <cmath>
/*
 * @lc app=leetcode.cn id=343 lang=cpp
 *
 * [343] 整数拆分
 */

// @lc code=start
class Solution {
public:
    int integerBreak(int n) {
        if (n <= 4) return std::pow(2, n - 2);
        std::div_t divResult = std::div(n, 3);
        return divResult.rem == 0 
            ? std::pow(3, divResult.quot)  
            : divResult.rem == 1 
                ? std::pow(3, divResult.quot - 1) * 4
                : std::pow(3, divResult.quot) * divResult.rem;
    }
};
// @lc code=end

class Solution {
public:
    int integerBreak(int n) {
        if (n <= 4) return std::pow(2, n - 2);
        std::vector<int> dp(n + 1, 0);
        dp[0] = 0, dp[1] = 0, dp[2] = 1, dp[3] = 2, dp[4] = 4;

        for (int i = 5; i <= n; ++i) {
            /*
                对于每个数字i，尝试所有可能的拆分方式
                j代表第一个拆分数，i-j代表剩余部分
                j从1遍历到i-1，考虑所有可能的拆分组合
            */
            for(int j = 1; j < i ; ++j) {
                dp[i] = std::max({j * (i - j), j * dp[i - j], dp[i]});
            }
        }
        return dp.back();
    }
};
