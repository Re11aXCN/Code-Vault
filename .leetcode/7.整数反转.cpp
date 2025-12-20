class Solution {
public:
    int reverse(int x) {
        bool isNegative{false};
        if(isNegative = x < 0) {
            if (x == INT_MIN) return 0;
            x = -x;
        }

        int result{0}, OVER_MAX = INT_MAX / 10;
        while(x > 0) {
            if (result > OVER_MAX) return 0;
            result = result * 10 + x % 10;
            x /= 10;
        }
        return isNegative ? -result : result;
    }
    
    int reverse(int x) {
        int result = 0;
        if (x < 10'0000'0000 && x > -10'0000'0000) {
            while(x != 0) {
                result = result * 10 + x % 10;
                x /= 10;
            }
            return result;
        }
        else {
            constexpr int int_max_div10 = INT_MAX / 10;
            bool isNegative = x < 0;
            if (isNegative) x = (x == INT_MIN) ? 0 : -x;
            while(x != 0) {
                int mod10 = x % 10;
                if (result > int_max_div10 && mod10 != 0) return 0;
                if (result == int_max_div10 && mod10 != 0) {
                    if (isNegative && mod10 > 8) return 0;
                    else if (mod10 > 7) return 0; 
                }
                result = result * 10 + mod10;
                x /= 10;
            }
            return isNegative ? -result : result;
        }
    }
};
/*
 * @lc app=leetcode.cn id=7 lang=cpp
 *
 * [7] 整数反转
 */

// @lc code=start
class Solution {
public:
    int reverse(int x) {
        int result = 0;
        while (x != 0) {
            // 检查是否会溢出
            if (result > INT_MAX / 10 || result < INT_MIN / 10) {
                return 0;
            }
            
            // 取出x的最后一位数字
            int digit = x % 10;
            
            // 将这一位添加到结果中
            result = result * 10 + digit;
            
            // 去掉x的最后一位
            x /= 10;
        }
        
        return result;
    }
};
// @lc code=end

