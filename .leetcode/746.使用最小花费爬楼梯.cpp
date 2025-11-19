/*
 * @lc app=leetcode.cn id=746 lang=cpp
 *
 * [746] 使用最小花费爬楼梯
 */

// @lc code=start
class Solution {
public:
    int minCostClimbingStairs(std::vector<int>& cost) {
        int prev2 = 0;  // dp[i-2]
        int prev1 = 0;  // dp[i-1]
        #pragma GCC unroll 8
        for (int i = 2; i <= cost.size(); ++i) {
            int current = std::min(prev1 + cost[i - 1], prev2 + cost[i - 2]);
            prev2 = prev1;
            prev1 = current;
        }
        
        return prev1;
    }
};
// @lc code=end
class Solution {
public:
    int minCostClimbingStairs(vector<int>& cost) {
        std::vector<int> dp(cost.size() + 1);
        dp[0] = 0, dp[1] = 0;
        for(int i = 2; i < dp.size(); ++i) {
            dp[i] = std::min(dp[i - 1] + cost[i - 1], dp[i - 2] + cost[i - 2]);
        }
        return dp.back();
    }
};

