/*
 * @lc app=leetcode.cn id=122 lang=cpp
 *
 * [122] 买卖股票的最佳时机 II
 */

// @lc code=start
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        int result{0};
        #pragma GCC unroll 8;
        for(int i = 1; i < prices.size(); ++i) {
            if(auto positive = prices[i] - prices[i - 1]; positive > 0) {
                result += positive;
            }
        }
        return result;
    }
};
// @lc code=end
    int maxProfit(vector<int>& prices) {
        int totalProfit = 0;
        int min_price = prices.front();
        int prev_price = min_price, curr_price{ -1 }; // 需要赋值处理prices.size() == 1的情况 即最后一天
        #pragma GCC unroll 8
        for (int i = 1; i < prices.size(); ++i) {
            prev_price = prices[i - 1];
            curr_price = prices[i];
            // 如果价格下跌，重置最小价格并累加之前的利润
            if (curr_price < prev_price) {
                totalProfit += prev_price - min_price;
                min_price = curr_price;
            }
        }
        // 最后一天的特殊处理
        if (curr_price >= prev_price) {
            totalProfit += curr_price - min_price;
        }
        
        return totalProfit;
    }

int maxProfit(vector<int>& prices) {
    int n = prices.size();
    if (n <= 1) return 0;
    
    // dp[i] 表示到第i天为止能获得的最大利润
    vector<int> dp(n, 0);
    // min_price[i] 表示到第i天为止遇到的最小价格
    vector<int> min_price(n, 0);
    
    // 初始化
    min_price[0] = prices[0];
    dp[0] = 0;
    
    for (int i = 1; i < n; i++) {
        // 更新最小价格
        min_price[i] = min(min_price[i-1], prices[i]);
        
        // 计算今天的利润选择：要么不操作，要么在今天卖出（如果有利可图）
        int profit_if_sell = prices[i] - min_price[i];
        
        if (profit_if_sell > 0) {
            // 如果今天卖出有利可图，我们选择卖出并重置最小价格
            dp[i] = dp[i-1] + profit_if_sell;
            min_price[i] = prices[i]; // 重置最小价格为当前价格
        } else {
            // 如果今天卖出无利可图，保持之前的利润
            dp[i] = dp[i-1];
        }
    }
    
    return dp[n-1];
}

int maxProfit(vector<int>& prices) {
    int not_hold_stock_profit = 0;           // 前一天结束时未持有股票的最大利润
    int hold_stock_profit = -prices.front(); // 前一天结束时持有股票的最大利润
    
    #pragma GCC unroll 8
    for(int i = 1; i < prices.size(); ++i) {
        int curr_price = prices[i];
        int prev = not_hold_stock_profit;

        int curr_profit;
        // 今天未持有：昨天未持有或今天卖出
        if (curr_profit = hold_stock_profit + curr_price;
            curr_profit > not_hold_stock_profit)
            not_hold_stock_profit = curr_profit;

        // 今天持有：昨天持有或今天买入
        if (curr_profit = prev - curr_price;
            curr_profit > hold_stock_profit)
            hold_stock_profit = curr_profit;  
    }
    return not_hold_stock_profit; // 最后一天，未持有股票才能获得最大利润
}
/*
买卖股票2（多次交易）
    可以买卖多次，所以买入时要考虑之前的利润
    dp[i - 1].no - prices[i] 表示用之前卖出股票获得的利润来买入新股
    这样可以实现"滚动投资"，用前一笔交易的利润进行下一笔投资
*/
int maxProfit(vector<int>& prices) {
    
    // 使用变量代替dp数组
    int dp0 = 0;           // 未持有股票的最大利润
    int dp1 = -prices[0];  // 持有股票的最大利润
    
    for (int i = 1; i < n; i++) {
        // 保存前一天的dp0，因为会被覆盖
        int prev_dp0 = dp0;
        
        // 更新当前状态
        dp0 = max(dp0, dp1 + prices[i]);  // 卖出
        dp1 = max(dp1, prev_dp0 - prices[i]); // 买入
    }
    
    return dp0;
}

int maxProfit(vector<int>& prices) {
    int n = prices.size();
    vector<vector<int>> dp(n, vector<int>(2));
    // dp[i][0]: 第i天结束时未持有股票的最大利润
    // dp[i][1]: 第i天结束时持有股票的最大利润
    
    dp[0][0] = 0;
    dp[0][1] = -prices[0];
    
    for(int i = 1; i < n; i++) {
        dp[i][0] = max(dp[i-1][0], dp[i-1][1] + prices[i]); // 卖出
        dp[i][1] = max(dp[i-1][1], dp[i-0][0] - prices[i]); // 买入
    }
    
    return dp[n-1][0];
}
