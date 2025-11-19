/*
 * @lc app=leetcode.cn id=392 lang=cpp
 *
 * [392] 判断子序列
 */

// @lc code=start
class Solution {
public:
    bool isSubsequence(string s, string t) {
        int s1Len = s.size(), s2Len = t.size();
        if (s1Len > s2Len) return false;

        int ptr1 = 0, ptr2 = 0;
        while (ptr1 < s1Len && ptr2 < s2Len)
        {
            if (s[ptr1] == t[ptr2]) ++ptr1;
            ++ptr2;
        }
        return ptr1 == s1Len;
    }
};
// @lc code=end

