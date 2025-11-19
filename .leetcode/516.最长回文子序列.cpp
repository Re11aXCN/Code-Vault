/*
 * @lc app=leetcode.cn id=516 lang=cpp
 *
 * [516] 最长回文子序列
 */

// @lc code=start
class Solution {
public:
    int longestPalindromeSubseq(string s) {
        std::vector<int> dp(s.size(), 0);
        
        for (int i = s.size() - 1; i >= 0; --i) {
            int bottomLeft = 0;  // 保存 dp[i+1][j-1] 的值
            dp[i] = 1;  // 对角线
            #pragma clang loop unroll_count(4)
            for (int j = i + 1; j < s.size(); ++j) {
                int bottom = dp[j];
                if (s[i] == s[j]) {
                    dp[j] = bottomLeft + 2;
                } else {
                    if(dp[j] < dp[j - 1]) dp[j] = dp[j - 1];
                }
                bottomLeft = bottom;
            }
        }
        
        return dp.back();
    }
};
// @lc code=end

