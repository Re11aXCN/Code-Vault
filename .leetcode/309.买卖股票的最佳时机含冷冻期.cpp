/*
 * @lc app=leetcode.cn id=309 lang=cpp
 *
 * [309] 买卖股票的最佳时机含冷冻期
 */

// @lc code=start
class Solution {
public:
int maxProfit(const std::vector<int>& prices) {
    int hold_stock = -prices[0];  // 持有股票
    int not_hold = 0;             // 不持有，非冷冻期
    int sell_today = 0;           // 今天卖出
    int cooldown = 0;             // 冷冻期
	#pragma clang loop unroll_count(8)
    for (int i = 1; i < prices.size(); ++i) {
        int prev_hold = hold_stock;
        int prev_not_hold = not_hold;
        int prev_sell = sell_today;
        int prev_cool = cooldown;

        // 更新当前状态
        hold_stock = std::max({
            prev_hold,
            prev_cool - prices[i],
            prev_not_hold - prices[i]
        });

        not_hold = std::max(prev_not_hold, prev_cool);
        sell_today = prev_hold + prices[i];
        cooldown = prev_sell;
    }

    return std::max({not_hold, sell_today, cooldown});
}
};
// @lc code=end

