/*
 * @lc app=leetcode.cn id=1143 lang=cpp
 *
 * [1143] 最长公共子序列
 */

// @lc code=start
class Solution {
public:
    int longestCommonSubsequence(string text1, string text2) {
 int text1Len = text1.length();
        int text2Len = text2.length();
        // 确保text1是较短的字符串，优化空间
        if(text1Len > text2Len) return longestCommonSubsequence(std::move(text2), std::move(text1));
        
        // 只使用一个数组
        std::vector<int> dp(text2Len + 1, 0);
        
        for(int i = 1; i <= text1Len; ++i) {
            int topLeft = dp[0];  // dp[i-1][j-1]
            for (int j = 1; j <= text2Len; j++) {
                int top = dp[j];  // 保存dp[i-1][j]
                dp[j] = text1[i - 1] == text2[j - 1] 
                    ? topLeft + 1 
                    : std::max(dp[j - 1], top); // 不相等的时候延续之前相等的状态
                topLeft = top; // 更新prev为原来的dp[i-1][j]
            }
        }
        
        return dp[text2Len];
    }
};
// @lc code=end

