/*
 * @lc app=leetcode.cn id=188 lang=cpp
 *
 * [188] 买卖股票的最佳时机 IV
 */

// @lc code=start
class Solution {
public:
int maxProfit(int k, vector<int>& prices) {
    std::vector<int> buy(k + 1, -prices[0]), sell(k + 1, 0);
    #pragma clang loog unroll_count(4)
    for(int i = 1; i < prices.size(); ++i) {
        #pragma clang loop interleave(enable) unroll_count(4)
        for(int j = 1; j <= k; ++j) {
            if (int val = sell[j - 1] - prices[i]; val > buy[j]) buy[j] = val;
            if (int val = buy[j] + price[i]; val > sell[j]) sell[j] = val;
        }
    }
    return sell.back();
}
};
// @lc code=end

