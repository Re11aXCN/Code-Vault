/*
 * @lc app=leetcode.cn id=583 lang=cpp
 *
 * [583] 两个字符串的删除操作
 */

// @lc code=start
class Solution {
public:
    int minDistance(string word1, string word2) {
        int w1Len = word1.size(), w2Len = word2.size();
        if (w1Len > w2Len) return minDistance(std::move(word2), std::move(word1));

        std::vector<int> dp(w2Len + 1, 0);
        #pragma clang loop unroll_count(8)
        for (int j = 0; j <= w2Len; ++j) dp[j] = j; // 操作第一行初始化为操作次数

        for(int i = 1; i <= word1.size(); ++i) {
            int topLeft = dp[0];
            dp[0] = i; // 更新当前行的第一个元素
            #pragma clang loop unroll_count(4)
            for(int j = 1; j <= word2.size(); ++j) {
                int top = dp[j];
                dp[j] = word1[i - 1] == word2[j - 1]
                    ? topLeft
                    : std::min({top + 1, dp[j - 1] + 1, topLeft + 2}); 
                // 可以优化为 (top + 1, dp[j - 1] + 1)，因为 dp[i][j - 1] = dp[i - 1][j - 1] + 1，优化之后比最长公共子序列思路快，否则慢
                topLeft = top;
            }
        }
        return dp.back();
    }
};
// @lc code=end
// 最长公共子序列思路
int minDistance(string word1, string word2) {
    int w1Len = word1.size(), w2Len = word2.size();
    if (w1Len > w2Len) return minDistance(std::move(word2), std::move(word1));

    std::vector<int> dp(w2Len + 1, 0);

    for(int i = 1; i <= word1.size(); ++i) {
        int topLeft = dp[0];
        #pragma clang loop unroll_count(4)
        for(int j = 1; j <= word2.size(); ++j) {
            int top = dp[j];
            dp[j] = word1[i - 1] == word2[j - 1]
                ? topLeft + 1
                : std::max(top, dp[j - 1]);
            topLeft = top;
        }
    }
    return w1Len + w2Len - dp.back() - dp.back();
}
