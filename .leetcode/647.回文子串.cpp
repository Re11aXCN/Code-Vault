/*
 * @lc app=leetcode.cn id=647 lang=cpp
 *
 * [647] 回文子串
 */

// @lc code=start
class Solution {
public:
    int countSubstrings(std::string s) {
        int str_len = static_cast<int>(s.size());
        int count = 0;

        // 遍历每个可能的中心点
        for (int center = 0; center < str_len; ++center) {
            // 奇数长度回文，以单个字符为中心
            int left = center;
            int right = center;

            #pragma GCC unroll 4
            while (left >= 0 && right < str_len && s[left] == s[right]) {
                ++count;
                --left;
                ++right;
            }

            // 偶数长度回文，以两个相同字符之间的空隙为中心
            left = center;
            right = center + 1;

            #pragma GCC unroll 4
            while (left >= 0 && right < str_len && s[left] == s[right]) {
                ++count;
                --left;
                ++right;
            }
        }

        return count;
    }
};
// @lc code=end

