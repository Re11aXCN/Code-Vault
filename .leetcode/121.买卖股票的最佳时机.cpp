// 交易k次通解
int maxProfit(vector<int>& prices) {
    int k = 2; // 交易k次
    vector<int> buy(k + 1, INT_MIN), sell(k + 1, 0);
    for (int price : prices) {
        for (int i = 1; i <= k; ++i) {
            buy[i] = max(buy[i], sell[i - 1] - price);
            sell[i] = max(sell[i], buy[i] + price);
        }
    }
    return sell[k];
}
/*
 * @lc app=leetcode.cn id=121 lang=cpp
 *
 * [121] 买卖股票的最佳时机
 */
// 动态规划
// @lc code=start
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        int min_price{ prices.front() }, max_profit{ 0 };
        #pragma GCC unroll 8
        for(auto it = std::next(prices.begin()); it != prices.end(); ++it) {
            if (*it < min_price) {
                min_price = *it;
                continue;
            }
            if (int curr_profit = *it - min_price; curr_profit > max_profit) max_profit = curr_profit;
        }
        return max_profit;
    }
};
// @lc code=end
// 动态规划状态压缩
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        int min_price{ prices.front() }, max_profit{ 0 };
        
        for (int i = 1; i < prices.size(); ++i) {
            min_price = min(min_price, prices[i]);
            max_profit = max(max_profit, prices[i] - min_price);
        }
        
        return max_profit;
    }
};
// 动态规划
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        int n = prices.size();
        if (n == 0) return 0;
        
        vector<int> dp(n, 0);  // dp[i]: 前i天的最大利润
        int minPrice = prices[0];
        
        for (int i = 1; i < n; i++) {
            minPrice = min(minPrice, prices[i]);
            dp[i] = max(dp[i-1], prices[i] - minPrice);
        }
        
        return dp[n-1];
    }
};
// 动态规划状态压缩
class Solution {
public:
int maxProfit(vector<int>& prices) {
    // 前一天结束时持有股票的最大利润, 前一天结束时未持有股票的最大利润
    int buy{ -prices.front() }, sell{ 0 };

    #pragma GCC unroll 8
    for(auto it = std::next(prices.begin()); it != prices.end(); ++it) {
        // 今天未持有：昨天未持有或今天卖出
        if (int val = sell - *it; val > buy) buy = val;
        // 今天持有：昨天持有或今天买入
        if (int val = buy + *it; val > sell) sell = val;
    }

    return sell;
}
};
// 动态规划
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        struct Profit {
            int sell{0};
            int buy{0};
        };
        std::vector<Profit> dp(prices.size());
        auto& [sell, buy] = dp.front();  
        buy = -prices.front();

        for(int i = 1; i < prices.size(); ++i) {
            auto [prev_sell, prev_buy] = dp[i - 1];
            auto& [curr_sell, curr_buy] = dp[i];
            int price = prices[i];
            curr_sell = std::max(prev_sell, prev_buy + price);
            curr_buy = std::max(prev_buy, -price);
        }
        return dp.back().sell;
    }
};
/*
买卖股票1（单次交易）
    只能买卖一次，所以买入时没有"先前状态"的概念
    买入时的成本就是 -prices[i]（从0现金开始）
    不能利用之前卖出股票的利润来再次买入
*/
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        int n = prices.size();
        if (n == 0) return 0;
        
        // dp[i][0]: 第i天不持有股票的最大利润
        // dp[i][1]: 第i天持有股票的最大利润
        vector<vector<int>> dp(n, vector<int>(2, 0));
        
        // 初始化
        dp[0][0] = 0;           // 第0天不持有股票
        dp[0][1] = -prices[0];  // 第0天持有股票（买入）
        
        for (int i = 1; i < n; i++) {
            // 第i天不持有股票：昨天就不持有，或者昨天持有今天卖出
            dp[i][0] = max(dp[i-1][0], dp[i-1][1] + prices[i]);
            // 第i天持有股票：昨天就持有，或者今天买入（注意：只能交易一次，所以是 -prices[i]）
            dp[i][1] = max(dp[i-1][1], -prices[i]);
        }
        
        return dp[n-1][0];  // 最后一天不持有股票的最大利润
    }
};
