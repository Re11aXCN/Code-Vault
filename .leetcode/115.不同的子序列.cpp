/*
 * @lc app=leetcode.cn id=115 lang=cpp
 *
 * [115] 不同的子序列
 */

// @lc code=start
class Solution {
public:
    int numDistinct(string s, string t) {
        int sLen = s.size(), tLen = t.size();
        if (sLen < tLen) return 0;
        
        // 使用一维数组，大小为 tLen + 1
        std::vector<std::size_t> dp(tLen + 1, 0);
        dp[0] = 1;  // 空字符串是任何字符串的子序列
        
        for (int i = 1; i <= sLen; ++i) {
            // 从后往前遍历，避免覆盖需要的数据
            #pragma clang loop unroll_count(8)
            // 内层循环从min(i, tLen)到1，减少不必要的计算，max确保剩余字符足够完成匹配
            for (int j = std::min(i, tLen); j >= std::max(tLen - sLen + i, 1); --j) {
                if (s[i - 1] == t[j - 1]) {
                    dp[j] = dp[j] + dp[j - 1];
                }
                // 不相等时，dp[j] 保持不变（相当于二维数组中的 dp[i-1][j]）
            }
        }
        return dp.back();
    }
};
// @lc code=end
    int numDistinct(string s, string t) {
        int s1Len = s.size(), s2Len = t.size();
        if (s1Len < s2Len) return 0;

        std::vector<std::vector<std::size_t>> dp(s1Len + 1, std::vector<std::size_t>(s2Len + 1, 0));
        for (int i = 0; i < s.size(); i++) dp[i][0] = 1;

        for(int i = 1; i <= s1Len; ++i) {
            for(int j = 1; j <= s2Len; ++j) {
                if (s[i - 1] == t[j - 1]) dp[i][j] = dp[i - 1][j - 1] + dp[i - 1][j];
                else dp[i][j] = dp[i - 1][j];
            }
        }
        return dp.back().back();
    }

    int numDistinct(string s, string t) {
        int s1Len = s.size(), s2Len = t.size();
        if (s1Len < s2Len) return 0;

        // 使用二维向量更清晰，或者正确计算一维索引
        std::vector<std::size_t> dp((s1Len + 1) * (s2Len + 1), 0);
        
        // 初始化：当t为空字符串时，s的任何子序列都包含1个空字符串
        #pragma clang loop unroll_count(8)
        for (int i = 0; i <= s1Len; ++i) {
            dp[i * (s2Len + 1)] = 1;  // dp[i][0] = 1
        }
        
        for(int i = 1; i <= s1Len; ++i) {
            #pragma clang loop unroll_count(4)
            for(int j = 1; j <= s2Len; ++j) {
                // 正确的索引计算：i * 列数 + j
                int idx = i * (s2Len + 1) + j;
                int prevIdx1 = (i - 1) * (s2Len + 1) + j;      // dp[i-1][j]
                int prevIdx2 = (i - 1) * (s2Len + 1) + (j - 1); // dp[i-1][j-1]
                
                if (s[i - 1] == t[j - 1]) {
                    dp[idx] = dp[prevIdx2] + dp[prevIdx1];
                } else {
                    dp[idx] = dp[prevIdx1];
                }
            }
        }
        
        return dp.back();//dp[s1Len * (s2Len + 1) + s2Len];  // dp[s1Len][s2Len]
    }
