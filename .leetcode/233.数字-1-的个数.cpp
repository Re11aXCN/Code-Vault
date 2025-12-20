/*
 * @lc app=leetcode.cn id=233 lang=cpp
 *
 * [233] 数字 1 的个数
 */

// @lc code=start
class Solution {
public:
    int countDigitOne(int n) {
#ifdef DP
        if (n <= 0) return 0;

        // 将数字转为字符串便于处理
        string s = to_string(n);
        int len = s.length();

        // dp[i] 表示从第i位开始到末尾的数字中1出现的次数
        vector<int> dp(len + 1, 0);

        // 从右往左计算
        for (int i = len - 1; i >= 0; i--) {
            int high = atoi(s.substr(0, i).c_str());  // 高位数字
            int low = (i + 1 < len) ? atoi(s.substr(i + 1).c_str()) : 0;  // 低位数字
            int digit = s[i] - '0';  // 当前位数字

            if (digit == 0) {
                // 当前位为0时，高位数字从0到high-1，每个对应10^(len-i-1)次
                dp[i] = high * pow(10, len - i - 1) + dp[i + 1];
            }
            else if (digit == 1) {
                // 当前位为1时，分两种情况：
                // 1. 高位从0到high-1
                // 2. 高位为high时，低位从0到low
                dp[i] = high * pow(10, len - i - 1) + (low + 1) + dp[i + 1];
            }
            else {
                // 当前位大于1时，高位从0到high
                dp[i] = (high + 1) * pow(10, len - i - 1) + dp[i + 1];
            }
        }

        return dp[0];
#else
        int count = 0;
        
        for (long long i = 1; i <= n; i *= 10) {
            long long divider = i * 10;
            // 当前位左边的数字
            long long left = n / divider;
            // 当前位的数字、当前位右边的数字
            auto [digit, right] = std::lldiv((long long)n, i);
            digit = digit % 10;
            
            // // 当前位右边的数字
            // long long right = n % i;
            // // 当前位的数字
            // long long digit = (n / i) % 10;
            
            if (digit == 0) {
                count += left * i;
            } else if (digit == 1) {
                count += left * i + right + 1;
            } else {
                count += (left + 1) * i;
            }
        }
        
        return count;
#endif
    }
};
// @lc code=end

