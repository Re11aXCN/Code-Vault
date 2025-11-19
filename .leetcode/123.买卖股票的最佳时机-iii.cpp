/*
第一次买入，在初始状态下买入，本金是0，所以 -prices.front()，第一次卖出 必须是买入了才能卖，所以又买又卖是0
第二次买入，在第一次买入再买（不符合题意在买入下一次之前必须卖出前一次），在第一次卖出再买，所以 -prices.front()，第二次卖出 必须是买入了才能卖，所以又买又卖是0
*/
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        int buy1{ -prices.front() }, sell1{ 0 };
        int buy2{ -prices.front() }, sell2{ 0 };

        #pragma GCC unroll 8
        for(auto it = std::next(prices.begin()); it != prices.end(); ++it) {
            if (-*it > buy1) buy1 = -*it;
            if (int val = buy1 + *it; val > sell1) sell1 = val;

            if (int val = sell1 - *it; val > buy2) buy2 = val;
            if (int val = buy2 + *it; val > sell2) sell2 = val;
        }

        return sell2;
    }
};

#include <vector>
#include <algorithm>
using namespace std;
/*
简单的两次贪心方法（先找第一笔最大利润交易，再在剩余部分找第二笔）确实无法得到最优解，
因为这种方法过于依赖第一笔交易的选择，可能破坏了全局最优组合。
然而，我们可以使用一种基于两次扫描的贪心动态规划方法，通过计算左右两个子数组的最大利润来确保找到最优解。
分割点动态规划

最优子结构
股票买卖问题具有最优子结构特性：
    整个问题的最优解包含子问题的最优解
    两笔交易的最大利润 = 前i天的最大利润 + 后n-i天的最大利润
无后效性
    前i天的最大利润只依赖于前i天的价格，与后面的价格无关
    后n-i天的最大利润只依赖于后n-i天的价格，与前面的价格无关
*/
int maxProfit(vector<int>& prices) {
    int n = prices.size();
    if (n <= 1) return 0;
    
    // 左数组：left[i] 表示 [0, i] 区间一次交易的最大利润
    vector<int> left(n, 0);
    int minPrice = prices[0];
    for (int i = 1; i < n; i++) {
        left[i] = max(left[i-1], prices[i] - minPrice);
        minPrice = min(minPrice, prices[i]);
    }
    
    // 右数组：right[i] 表示 [i, n-1] 区间一次交易的最大利润
    vector<int> right(n, 0);
    int maxPrice = prices[n-1];
    for (int i = n-2; i >= 0; i--) {
        right[i] = max(right[i+1], maxPrice - prices[i]);
        maxPrice = max(maxPrice, prices[i]);
    }
    
    // 寻找最大组合利润
    int maxProfit = 0;
    for (int i = 0; i < n; i++) {
        maxProfit = max(maxProfit, left[i] + right[i]);
    }
    return maxProfit;
}

// 空间优化
int maxProfit(vector<int>& prices) {
    int n = prices.size();
    if (n <= 1) return 0;
    
    // 计算左数组的同时直接计算最大组合利润
    vector<int> right(n, 0);
    int maxPrice = prices[n-1];
    for (int i = n-2; i >= 0; i--) {
        right[i] = max(right[i+1], maxPrice - prices[i]);
        maxPrice = max(maxPrice, prices[i]);
    }
    
    int maxProfit = right[0]; // 至少等于一次交易利润
    int minPrice = prices[0];
    int leftProfit = 0;
    for (int i = 1; i < n; i++) {
        leftProfit = max(leftProfit, prices[i] - minPrice);
        minPrice = min(minPrice, prices[i]);
        maxProfit = max(maxProfit, leftProfit + right[i]);
    }
    return maxProfit;
}

int maxProfit(vector<int>& prices) {
    int n = prices.size();
    if (n <= 1) return 0;
    
    // 第一趟：计算所有可能的第一笔交易利润
    vector<int> firstProfit(n, 0);
    int minPrice = prices[0];
    for (int i = 1; i < n; i++) {
        firstProfit[i] = max(firstProfit[i-1], prices[i] - minPrice);
        minPrice = min(minPrice, prices[i]);
    }
    
    // 第二趟：对于每个可能的分割点，计算第二笔交易的最大利润
    int maxTotalProfit = firstProfit[n-1]; // 至少等于一次交易利润
    int maxPrice = prices[n-1];
    int secondProfit = 0;
    
    for (int i = n-2; i >= 0; i--) {
        // 计算从i到n-1的第二笔交易最大利润
        secondProfit = max(secondProfit, maxPrice - prices[i]);
        maxPrice = max(maxPrice, prices[i]);
        
        // 组合第一笔交易（0到i）和第二笔交易（i到n-1）
        int totalProfit = firstProfit[i] + secondProfit;
        maxTotalProfit = max(maxTotalProfit, totalProfit);
    }
    
    return maxTotalProfit;
}

/*
 * @lc app=leetcode.cn id=123 lang=cpp
 *
 * [123] 买卖股票的最佳时机 III
 */

// @lc code=start
#include <vector>
using namespace std;
class Solution {
public:

    //不断递增只需要处理首尾部股票，买卖一次
    //一旦有落差，买卖两次
    //思考不全面1,2,4,2,5,7,2,4,9,0
    /*
    int maxProfit(vector<int>& prices) {
        int max1 = 0, max2 = 0;
        int start = 0;
        int n = prices.size();
        int increase_count = 0, decrease_count = 0;
        for(int i = 1; i < prices.size(); ++i){
            if(prices[i] > prices[i - 1]){
                increase_count++;
            }
            else {
                if(prices[i] < prices[i - 1]) decrease_count++;
                // 当前递增区段结束于i-1，计算差值
                int length = i - 1 - start + 1;
                if (length >= 2) {
                    int diff = prices[i - 1] - prices[start];
                    if (diff > max1) {
                        max2 = max1;
                        max1 = diff;
                    }
                    else if (diff > max2) {
                        max2 = diff;
                    }
                }
                start = i; // 新递增区段开始
            }
        }
        // 处理最后一个区段
        int length = n - start;
        if (length >= 2) {
            int diff = prices.back() - prices[start];
            if (diff > max1) {
                max2 = max1;
                max1 = diff;
            }
            else if (diff > max2) {
                max2 = diff;
            }
        }
        if(increase_count == prices.size() - 1) return prices.back() - prices.front();
        else if(decrease_count == prices.size() - 1) return 0;
        else return max1 + max2;
    }
    */
   /*
状态转移过程
    第一次买入：比较之前的最低成本（firstBuy）和当天买入的成本（-prices[i]），选择更小的成本。
    第一次卖出：比较之前的最高利润（firstSell）和当天卖出的利润（firstBuy + prices[i]），选择更大利润。
    第二次买入：比较之前的第二次买入成本（secondBuy）和当天买入的成本（firstSell - prices[i]），选择更小的成本。
    第二次卖出：比较之前的最高利润（secondSell）和当天卖出的利润（secondBuy + prices[i]），选择更大利润。
   */
    int maxProfit(vector<int>& prices) {

        // 定义状态变量
        // buy1: 第一次买入后的最大利润
        // sell1: 第一次卖出后的最大利润
        // buy2: 第二次买入后的最大利润
        // sell2: 第二次卖出后的最大利润
        int buy1 = -prices[0], sell1 = 0;
        int buy2 = -prices[0], sell2 = 0;

        for (int i = 1; i < prices.size(); ++i) {
            // 第一次买入：前一天已买入，或当天买入
            buy1 = max(buy1, -prices[i]);

            // 第一次卖出：前一天已卖出，或当天卖出
            sell1 = max(sell1, buy1 + prices[i]);

            // 第二次买入：前一天已第二次买入，或当天第二次买入（使用第一次卖出的利润）
            buy2 = max(buy2, sell1 - prices[i]);

            // 第二次卖出：前一天已第二次卖出，或当天第二次卖出
            sell2 = max(sell2, buy2 + prices[i]);
        }

        // 最终的最大利润是完成两次交易后的利润
        return sell2;
    }
};
// @lc code=end
int main(){
    Solution s;
    vector<int> prices = { 1,2,4,2,5,7,2,4,9,0 };
    auto res = s.maxProfit(prices);
    return 0;
}



/*
    0 不操作
    1 第一次持有
    2 第一次不持有
    3 第二次持有
    4 第二次不持有
*/
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        std::vector<std::array<int, 5>> dp(prices.size(), std::array<int, 5>{});
        dp[0][0] = 0;
        dp[0][1] = -prices[0];
        dp[0][2] = 0;
        dp[0][3] = -prices[0];
        dp[0][4] = 0;

        for(int i = 1; i < prices.size(); ++i) {
            dp[i][1] = max(dp[i - 1][1], dp[i - 1][0] - prices[i]);
            dp[i][2] = max(dp[i - 1][2], dp[i - 1][1] + prices[i]);

            dp[i][3] = max(dp[i - 1][3], dp[i - 1][2] - prices[i]);
            dp[i][4] = max(dp[i - 1][4], dp[i - 1][3] + prices[i]);
        }
        return dp[prices.size() - 1][4];
    }
};















// 两趟贪心
// 第一趟贪心一旦下降就计算并维护left、right区间
// 第二趟贪心和121一样
// 当前代码的反例[2,1,4,5,2,9,7]，两趟贪心13，正确是11
int maxProfit(vector<int>& prices) {
    int n = prices.size();
    if (n <= 1) return 0;

    // 第一趟贪心：找到最大利润交易区间
    int minPrice = prices[0];
    int currentStart = 0;
    int maxProfit1 = 0;
    int left1 = 0, right1 = 0;

    for (int i = 1; i < n; i++) {
        if (prices[i] < prices[i - 1]) {
            // 价格下降，计算当前段利润
            int profit = prices[i - 1] - minPrice;
            if (profit > maxProfit1) {
                maxProfit1 = profit;
                left1 = currentStart;
                right1 = i - 1;
            }
            // 重置最小价格和起始点
            minPrice = prices[i];
            currentStart = i;
        }
    }
    // 遍历结束后，计算最后一段利润
    int finalProfit = prices[n - 1] - minPrice;
    if (finalProfit > maxProfit1) {
        maxProfit1 = finalProfit;
        left1 = currentStart;
        right1 = n - 1;
    }

    // 第二趟贪心：跳过第一趟区间，在剩余部分计算一次交易最大利润
    int maxProfit2 = 0;
    int minPrice2 = INT_MAX;
    for (int i = 0; i < n; i++) {
        if (i >= left1 && i <= right1) continue; // 跳过第一趟区间
        if (prices[i] < minPrice2) {
            minPrice2 = prices[i];
        } else if (prices[i] - minPrice2 > maxProfit2) {
            maxProfit2 = prices[i] - minPrice2;
        }
    }

    return maxProfit1 + maxProfit2;
}

//////////////////////////////////////////////// 改进一
// [8,3,6,2,8,8,8,4,2,0,7,2,9,4,9]反例 最终结果是14  正确结果是15

// 辅助函数：计算prices子数组 [start, end] 的一次交易最大利润（121题贪心）
int oneTransactionProfit(vector<int>& prices, int start, int end) {
    if (start > end) return 0;
    int minPrice = prices[start];
    int maxProfit = 0;
    for (int i = start + 1; i <= end; i++) {
        if (prices[i] < minPrice) {
            minPrice = prices[i];
        } else if (prices[i] - minPrice > maxProfit) {
            maxProfit = prices[i] - minPrice;
        }
    }
    return maxProfit;
}

int maxProfit(vector<int>& prices) {
    int n = prices.size();
    if (n <= 1) return 0;

    // 第一趟贪心：找到最大利润交易区间
    int minPrice = prices[0];
    int currentStart = 0;
    int maxProfit1 = 0;
    int left1 = 0, right1 = 0;

    for (int i = 1; i < n; i++) {
        if (prices[i] < prices[i - 1]) {
            // 价格下降，计算当前段利润
            int profit = prices[i - 1] - minPrice;
            if (profit > maxProfit1) {
                maxProfit1 = profit;
                left1 = currentStart;
                right1 = i - 1;
            }
            // 重置最小价格和起始点
            minPrice = prices[i];
            currentStart = i;
        }
    }
    // 遍历结束后，计算最后一段利润
    int finalProfit = prices[n - 1] - minPrice;
    if (finalProfit > maxProfit1) {
        maxProfit1 = finalProfit;
        left1 = currentStart;
        right1 = n - 1;
    }

    // 第二趟贪心：在第一趟区间之前和之后分别计算一次交易最大利润
    int profitBefore = oneTransactionProfit(prices, 0, left1 - 1);
    int profitAfter = oneTransactionProfit(prices, right1 + 1, n - 1);
    int maxProfit2 = max(profitBefore, profitAfter);

    return maxProfit1 + maxProfit2;
}

//////////////////////////////////////////////// 改进二
/*
[397,6621,4997,7506,8918,1662,9187,3278,3890,514,18,9305,93,5508,3031,2692,6019,1134,1691,4949,5071,799,8953,7882,4273,302,6753,4657,8368,3942,1982,5117,563,3332,2623,9482,4994,8163,9112,5236,5029,5483,4542,1474,991,3925,4166,3362,5059,5857,4663,6482,3008,3616,4365,3634,270,1118,8291,4990,1413,273,107,1976,9957,9083,7810,4952,7246,3275,6540,2275,8758,7434,3750,6101,1359,4268,5815,2771,126,478,9253,9486,446,3618,3120,7068,1089,1411,2058,2502,8037,2165,830,7994,1248,4993,9298,4846,8268,2191,3474,3378,9625,7224,9479,985,1492,1646,3756,7970,8476,3009,7457,8922,2980,577,2342,4069,8341,4400,2923,2730,2917,105,724,518,5098,6375,5364,3366,8566,8838,3096,8191,2414,2575,5528,259,573,5636,4581,9049,4998,2038,4323,7978,8968,6665,8399,7309,7417,1322,6391,335,1427,7115,853,2878,9842,2569,2596,4760,7760,5693,9304,6526,8268,4832,6785,5194,6821,1367,4243,1819,9757,4919,6149,8725,7936,4548,2386,5354,2222,8777,2041,1,2245,9246,2879,8439,1815,5476,3200,5927,7521,2504,2454,5789,3688,9239,7335,6861,6958,7931,8680,3068,2850,1181,1793,7138,2081,532,2492,4303,5661,885,657,4258,131,9888,9050,1947,1716,2250,4226,9237,1106,6680,1379,1146,2272,8714,8008,9230,6645,3040,2298,5847,4222,444,2986,2655,7328,1830,6959,9341,2716,3968,9952,2847,3856,9002,1146,5573,1252,5373,1162,8710,2053,2541,9856,677,1256,4216,9908,4253,3609,8558,6453,4183,5354,9439,6838,2682,7621,149,8376,337,4117,8328,9537,4326,7330,683,9899,4934,2408,7413,9996,814,9955,9852,1491,7563,421,7751,1816,4030,2662,8269,8213,8016,4060,5051,7051,1682,5201,5427,8371,5670,3755,7908,9996,7437,4944,9895,2371,7352,3661,2367,4518,3616,8571,6010,1179,5344,113,9347,9374,2775,3969,3939,792,4381,8991,7843,2415,544,3270,787,6214,3377,8695,6211,814,9991,2458,9537,7344,6119,1904,8214,6087,6827,4224,7266,2172,690,2966,7898,3465,3287,1838,609,7668,829,8452,84,7725,8074,871,3939,7803,5918,6502,4969,5910,5313,4506,9606,1432,2762,7820,3872,9590,8397,1138,8114,9087,456,6012,8904,3743,7850,9514,7764,5031,4318,7848,9108,8745,5071,9400,2900,7341,5902,7870,3251,7567,2376,9209,9000,1491,7030,2872,7433,1779,362,5547,7218,7171,7911,2474,914,2114,8340,8678,3497,2659,2878,2606,7756,7949,2006,656,5291,4260,8526,4894,1828,7255,456,7180,8746,3838,6404,6179,5617,3118,8078,9187,289,5989,1661,1204,8103,2,6234,7953,9013,5465,559,6769,9766,2565,7425,1409,3177,2304,6304,5005,9559,6760,2185,4657,598,8589,836,2567,1708,5266,1754,8349,1255,9767,5905,5711,9769,8492,3664,5134,3957,575,1903,3723,3140,5681,5133,6317,4337,7789,7675,3896,4549,6212,8553,1499,1154,5741,418,9214,1007,2172,7563,8614,8291,3469,677,4413,1961,4341,9547,5918,4916,7803,9641,4408,3484,1126,7078,7821,8915,1105,8069,9816,7317,2974,1315,8471,8715,1733,7685,6074,257,5249,4688,8549,5070,5366,2962,7031,6059,8861,9301,7328,6664,5294,8088,6500,6421,1518,4321,5336,2623,8742,1505,9941,1716,2820,4764,6783,906,2450,2857,7515,4051,7546,2416,9121,9264,1730,6152,1675,592,1805,9003,7256,7099,3444,3757,9872,4962,4430,1561,7586,3173,3066,3879,1241,2238,8643,8025,3144,7445,882,7012,1496,4780,9428,617,396,1159,3121,2072,1751,4926,7427,5359,8378,871,5468,8250,5834,9899,9811,9772,9424,2877,3651,7017,5116,8646,5042,4612,6092,2277,1624,7588,3409,1053,8206,3806,8564,7679,2230,6667,8958,6009,2026,7336,6881,3847,5586,9067,98,1750,8839,9522,4627,8842,2891,6095,7488,7934,708,3580,6563,8684,7521,9972,6089,2079,130,4653,9758,2360,1320,8716,8370,9699,6052,1603,3546,7991,670,3644,6093,9509,9518,7072,4703,2409,3168,2191,6695,228,2124,3258,5264,9645,9583,1354,1724,9713,2359,1482,8426,3680,6551,3148,9731,8955,4751,9629,6946,5421,9625,9391,1282,5495,6464,5985,4256,5984,4528,952,6212,6652,562,1476,6297,145,9182,8021,6211,1542,5856,4637,1574,2407,7785,1305,1362,2536,934,4661,4309,559,4052,1943,2406,516,4280,6662,2852,8808,7614,9064,1813,4529,6893,8110,4674,2427,2484,7237,3969,8340,1874,5543,7099,6011,3200,8461,8547,486,9474,9208,7397,9879,7503,9803,6747,1783,6466,9600,6944,432,8664,8757,4961,1909,6867,5988,4337,5703,3225,4658,4043,1452,6554,1142,7463,9754,5956,2363,241,1782,7923,7638,1661,5427,3794,8409,7210,260,8009,4154,692,3025,9263,2006,4935,2483,7994,5624,8186,7571,282,8582,9023,6836,6076,6487,6591,2032,8850,3184,3815,3125,7174,5476,8552,968,3885,2115,7580,8246,2621,4625,1272,1885,6631,6207,4368,4625,8183,2554,8548,8465,1136,7572,1654,7213,411,4597,5597,5613,7781,5764,8738,1307,7593,7291,8628,7830,9406,6208,6077,2027,833,7349,3912,7464,9908,4632,8441,8091,7187,6990,2908,4675,914,4562,8240,1325,9159,190,6938,3292,5954,2028,4600,9899,9319,3228,7730,5077,9436,159,7105,6622,7508,7369,4086,3768,2002,8880,8211,5541,2222,1119,216,3136,5682,4809,813,1193,4999,4103,4486,7305,6131,9086,7205,5451,2314,1287,528,8102,1446,3985,4724,5306,1355,5163,9074,9709,4043,7285,5250,2617,4756,1818,2105,6790,6627,2918,7984,7978,7021,2470,1636,3152,7908,8841,4955,222,6480,5484,4676,7926,5821,9401,3232,7176,916,8658,3237,1311,5943,8487,3928,7051,306,6033,3842,3285,8951,1826,7616,2324,648,9252,5476,8556,4445,6784]

这个是一个反例，两次贪心结果是19845，正确结果是19965
*/
int maxProfit(vector<int>& prices) {
    int n = prices.size();
    if (n <= 1) return 0;

    // 第一趟贪心：找到所有可能的利润区间
    vector<pair<pair<int, int>, int>> candidates; // 存储((left, right), profit)
    int minPrice = prices[0];
    int currentStart = 0;

    for (int i = 1; i < n; i++) {
        if (prices[i] < prices[i - 1]) {
            // 价格下降，计算当前段利润
            int profit = prices[i - 1] - minPrice;
            if (profit > 0) {
                candidates.push_back({{currentStart, i - 1}, profit});
            }
            // 重置最小价格和起始点
            minPrice = prices[i];
            currentStart = i;
        }
    }
    // 遍历结束后，计算最后一段利润
    int finalProfit = prices[n - 1] - minPrice;
    if (finalProfit > 0) {
        candidates.push_back({{currentStart, n - 1}, finalProfit});
    }
    
    // 如果没有找到任何候选区间，返回0
    if (candidates.empty()) return 0;
    
    // 按利润降序排序候选区间
    sort(candidates.begin(), candidates.end(), 
         [](const pair<pair<int, int>, int>& a, const pair<pair<int, int>, int>& b) {
             return a.second > b.second;
         });
    
    // 考虑前k个候选区间，避免过多计算
    int k = min(5, (int)candidates.size());
    int maxTotalProfit = 0;
    
    for (int i = 0; i < k; i++) {
        int left1 = candidates[i].first.first;
        int right1 = candidates[i].first.second;
        int profit1 = candidates[i].second;
        
        // 第二趟贪心：在第一趟区间之前和之后分别计算一次交易最大利润
        int profitBefore = oneTransactionProfit(prices, 0, left1 - 1);
        int profitAfter = oneTransactionProfit(prices, right1 + 1, n - 1);
        int profit2 = max(profitBefore, profitAfter);
        
        maxTotalProfit = max(maxTotalProfit, profit1 + profit2);
    }
    
    return maxTotalProfit;
}