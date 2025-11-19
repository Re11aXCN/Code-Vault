/*
 * @lc app=leetcode.cn id=9 lang=cpp
 *
 * [9] 回文数
 */

// @lc code=start
#include <string>
class Solution {
public:
    bool isPalindrome(int x) {
        // 特殊情况：
        // 1. 负数不可能是回文数
        // 2. 如果数字的最后一位是0，则第一位也必须是0，即只有0满足
        if(x < 0 || (x % 10 == 0 && x != 0)) return false;
#define OPTIMIZATION
#ifdef STRING
        std::string s = std::to_string(x);
        for(int left = 0, right = s.size() - 1; left < right; ++left, --right)
        {
            if(s[left] != s[right]) return false;
        }
        return true;
#else
#ifdef OPTIMIZATION
    int reversedHalf = 0;
    // 只反转一半的数字
    // 当原始数字小于或等于反转后的数字时，说明已经处理了一半或一半以上的数字
    while(x > reversedHalf) {
        reversedHalf = reversedHalf * 10 + x % 10;
        x /= 10;
    }
    
    // 对于偶数位数的回文数，x和reversedHalf应该相等
    // 对于奇数位数的回文数，reversedHalf的最后一位是x的中间数字，需要去掉
    return x == reversedHalf || x == reversedHalf / 10;
#else
    int reverse = 0;
    int original = x;
    while (x != 0) {
        // 检查是否会溢出
        if (reverse > INT_MAX / 10 || reverse < INT_MIN / 10) {
            return 0;
        }
        
        // 取出x的最后一位数字
        int digit = x % 10;
        
        // 将这一位添加到结果中
        reverse = reverse * 10 + digit;
        
        // 去掉x的最后一位
        x /= 10;
    }
    return (original - reverse) == 0;
#endif
#endif
#undef OPTIMIZATION
    }
};
// @lc code=end
int main() {
    Solution s;
    auto result = s.isPalindrome(121);
    return 0;
}

