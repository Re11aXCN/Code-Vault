/*
 * @lc app=leetcode.cn id=5 lang=cpp
 *
 * [5] 最长回文子串
 */

// @lc code=start
/*
## 中心扩展法
### 原理
中心扩展法的核心思想是： 回文串都是基于中心对称的 。因此，我们可以枚举所有可能的回文中心，然后向两边扩展，直到不能形成回文为止。

需要注意的是，回文串的长度可能是奇数也可能是偶数：

- 奇数长度的回文串中心是一个字符，如 "aba" 的中心是 'b'
- 偶数长度的回文串中心是两个字符之间的位置，如 "abba" 的中心是 'b' 和 'b' 之间
因此，对于长度为n的字符串，我们需要考虑2n-1个中心位置。

## Manacher算法（最优解）
### 原理
Manacher算法是专门用于查找最长回文子串的线性时间算法，其核心思想是 利用已经计算过的回文信息，避免重复计算 。

算法的关键步骤：

1. 预处理字符串 ：在原字符串的每个字符之间插入一个特殊字符（通常是'#'），这样可以统一处理奇数和偶数长度的回文串。
2. 维护一个回文半径数组P ：P[i]表示以第i个字符为中心的最长回文子串的半径。
3. 利用对称性加速计算 ：如果当前位置i在某个已知回文子串的范围内，可以利用对称位置的回文信息来加速计算。
4. 中心扩展 ：在利用对称性后，仍然需要尝试向两边扩展，以找到可能更长的回文。

## 动态规划法（非最优但易理解）
### 原理
动态规划的思路是：定义状态dp[i][j]表示子串s[i...j]是否为回文串。状态转移方程为：

- 当s[i] == s[j]时，dp[i][j] = dp[i+1][j-1]（如果i+1 <= j-1）
- 当s[i] != s[j]时，dp[i][j] = false
边界条件：

- 当i == j时，dp[i][j] = true（单个字符是回文）
- 当j = i+1时，dp[i][j] = (s[i] == s[j])（两个字符是否相同）
*/
#define Manacher
//#define DP
//#define Center_Expend
class Solution {
public:
    string longestPalindrome(string s) {
#ifdef Center_Expend
        if (s.size() < 2) return s;
        
        int start = 0, maxLen = 1;
        
        // 枚举所有可能的回文中心
        for (int i = 0; i < s.size(); i++) {
            // 以s[i]为中心的奇数长度回文串
            expandAroundCenter(s, i, i, start, maxLen);
            // 以s[i]和s[i+1]之间为中心的偶数长度回文串
            expandAroundCenter(s, i, i + 1, start, maxLen);
        }
        
        return s.substr(start, maxLen);
#elif defined(DP)
        int n = s.size();
        if (n < 2) return s;
        
        // dp[i][j]表示s[i...j]是否为回文串
        vector<vector<bool>> dp(n, vector<bool>(n, false));
        
        int start = 0;  // 最长回文子串的起始位置
        int maxLen = 1;  // 最长回文子串的长度
        
        // 所有单个字符都是回文
        for (int i = 0; i < n; i++) {
            dp[i][i] = true;
        }
        
        // 检查长度为2的子串
        for (int i = 0; i < n - 1; i++) {
            if (s[i] == s[i + 1]) {
                dp[i][i + 1] = true;
                start = i;
                maxLen = 2;
            }
        }
        
        // 检查长度大于2的子串
        for (int len = 3; len <= n; len++) {  // 子串长度
            for (int i = 0; i <= n - len; i++) {  // 子串起始位置
                int j = i + len - 1;  // 子串结束位置
                
                // 状态转移方程
                if (s[i] == s[j] && dp[i + 1][j - 1]) {
                    dp[i][j] = true;
                    start = i;
                    maxLen = len;
                }
            }
        }
        
        return s.substr(start, maxLen);
#elif defined(Manacher)
        if (s.length() < 2) return s;

        string mana_str((s.length() << 1) + 3, '#');
        mana_str[0] = '^';
        mana_str.back() = '$';
        for (size_t i = 0; i < s.length(); ++i) {
            mana_str[(i + 1) << 1] = s[i]; // 保证c处在的位置都是偶数
        }
        vector<size_t> P(mana_str.length(), 0); // 回文半径数组
        // 当前回文中心, 当前回文右边界
        size_t C{ 0 }, R{ 0 };
        // 最长回文子串的长度, 最长回文子串的中心位置
        size_t maxLen{ 0 }, centerIndex{ 0 };

        for (size_t i = 1; i < mana_str.length() - 1; ++i) {
            // 利用对称性
            // 当前i在已知的最大覆盖范围R内，取R-i和其中心镜像位置最小值
            if (i <= R) P[i] = std::min(R - i, P[(C << 1) - i]);
            // 中心扩展
            while (mana_str[i - P[i] - 1] == mana_str[i + P[i] + 1]) ++P[i];

            // 更新C和R
            // 当前蘑菇最右侧比已知最大蘑菇最右侧还大
            if (P[i] + i > R) {
                R = P[i] + i;
                C = i;
            }

            // 更新最长回文子串信息
            if (P[i] > maxLen) {
                maxLen = P[i];
                centerIndex = i;
            }
        }
        // 除以2是因为添加了'#'
        return s.substr((centerIndex - maxLen) >> 1, maxLen);
#endif
    }
    
private:
    // 从中心向两边扩展，寻找最长回文子串
    void expandAroundCenter(const string& s, int left, int right, int& start, int& maxLen) {
        // 向两边扩展，直到不能形成回文
        while (left >= 0 && right < s.size() && s[left] == s[right]) {
            left--;
            right++;
        }
        
        // 计算回文长度（注意left和right已经各自多走了一步）
        int len = right - left - 1;
        
        // 更新最长回文子串的起始位置和长度
        if (len > maxLen) {
            start = left + 1;
            maxLen = len;
        }
    }
};
// @lc code=end

class Solution {
public:
    string longestPalindrome(string s) {
        struct StringView { 
            int begin, end; 
            int length() const { return end - begin; }
        };

        if (s.empty()) return "";
        
        std::vector<StringView> dp(s.size());
        
        // 初始化：每个单个字符都是回文
        for(int i = 0; i < s.size(); ++i) {
            dp[i] = {i, i + 1};
        }
        
        StringView longest = dp[0]; // 记录最长回文子串
        
        for(int i = s.size() - 2; i >= 0; --i) { // 从倒数第二个开始
            StringView bottomLeft = dp[i + 1]; // 保存dp[i+1][j-1]
            #pragma clang loop interleave(enable) unroll_count(4)
            for(int j = i + 1; j < s.size(); ++j) {
                StringView currentBottom = dp[j]; // 保存当前dp[j]，即dp[i+1][j]
                StringView currentLeft = (j > i + 1) ? dp[j - 1] : StringView{j, j}; // dp[i][j-1]
                
                if(s[i] == s[j]) {
                    // 如果两端字符相等，检查内部子串是否是回文
                    if(j - i <= 2 || (bottomLeft.begin == i + 1 && bottomLeft.end == j)) {
                        dp[j] = {i, j + 1};
                    } else {
                        // 内部不是回文，取左右子串中较长的
                        if(currentLeft.length() > currentBottom.length()) {
                            dp[j] = currentLeft;
                        } else {
                            dp[j] = currentBottom;
                        }
                    }
                } else {
                    // 两端字符不相等，取左右子串中较长的
                    if(currentLeft.length() > currentBottom.length()) {
                        dp[j] = currentLeft;
                    } else {
                        dp[j] = currentBottom;
                    }
                }
                
                // 更新最长回文子串
                if(dp[j].length() > longest.length()) {
                    longest = dp[j];
                }
                
                bottomLeft = currentBottom; // 更新bottomLeft为当前行的dp[j-1]
            }
        }
        
        return s.substr(longest.begin, longest.length());
    }
};
