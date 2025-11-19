/*
 * @lc app=leetcode.cn id=415 lang=cpp
 *
 * [415] 字符串相加
 */


// @lc code=start
#include <unordered_map>
#include <string>
#include <algorithm>
class Solution {
#ifdef MAP
    inline static std::unordered_map<int, char> ASCII_INT_ADD_CARRY_MAP = {
        {96, '0'}, {97, '1'}, {98, '2'}, {99, '3'}, {100, '4'},
        {101, '5'}, {102, '6'}, {103, '7'}, {104, '8'}, {105, '9'},
        {106, '0'}, {107, '1'}, {108, '2'}, {109, '3'}, {110, '4'},
        {111, '5'}, {112, '6'}, {113, '7'}, {114, '8'}, {115, '9'}
    };
#endif
public:
    std::string addStrings(std::string num1, std::string num2) {
        // 确保num1是较长的字符串
        if (num1.length() < num2.length()) {
            std::swap(num1, num2);
        }
        
        int len1 = num1.length();
        int len2 = num2.length();
        int carry = 0;
        
        // 预分配结果，多一位用于可能的进位
        std::string result(len1 + 1, '0');
        
        int i = len1 - 1, j = len2 - 1, k = len1;
        
        while (i >= 0 || j >= 0 || carry) {
            char a = (i >= 0) ? num1[i] : '0';
            char b = (j >= 0) ? num2[j] : '0';
            int ascii_sum = a + b + carry;
#ifdef MAP
            result[k] = ASCII_INT_ADD_CARRY_MAP[ascii_sum];
            carry = (ascii_sum >= 106) ? 1 : 0;
#else
            int dec = ascii_sum - 2 * '0'; 
            result[k] = '0' + (dec % 10);  // 当前位结果
            carry = dec / 10;               // 进位
#endif
            i--; j--; k--;
        }
        
        // 如果最高位没有进位，去掉前导零
        if (result[0] == '0') {
            return result.substr(1);
        }
        
        return result;
    }
};
// @lc code=end
#include <string>
#include <algorithm>
#include <iostream>

class Solution {
private:
    // 比较两个正数字符串的大小
    bool isGreaterOrEqual(const std::string& num1, const std::string& num2) {
        if (num1.length() != num2.length()) {
            return num1.length() > num2.length();
        }
        return num1 >= num2;
    }
    
    // 正数字符串相减 (num1 >= num2)
    std::string subtractPositiveStrings(const std::string& num1, const std::string& num2) {
        if (num1 == num2) return "0";
        
        std::string result;
        int borrow = 0;
        int i = num1.length() - 1, j = num2.length() - 1;
        
        while (i >= 0 || j >= 0) {
            int digit1 = (i >= 0) ? num1[i] - '0' : 0;
            int digit2 = (j >= 0) ? num2[j] - '0' : 0;
            
            int diff = digit1 - digit2 - borrow;
            if (diff < 0) {
                diff += 10;
                borrow = 1;
            } else {
                borrow = 0;
            }
            
            result.push_back('0' + diff);
            i--; j--;
        }
        
        // 去除前导零
        while (result.length() > 1 && result.back() == '0') {
            result.pop_back();
        }
        
        std::reverse(result.begin(), result.end());
        return result;
    }
    
    // 正数字符串相加（您原来的实现）
    std::string addPositiveStrings(const std::string& num1, const std::string& num2) {
        if (num1.length() < num2.length()) {
            std::swap(num1, num2);
        }
        
        int len1 = num1.length();
        int len2 = num2.length();
        int carry = 0;
        
        std::string result(len1 + 1, '0');
        int i = len1 - 1, j = len2 - 1, k = len1;
        
        while (i >= 0 || j >= 0 || carry) {
            char a = (i >= 0) ? num1[i] : '0';
            char b = (j >= 0) ? num2[j] : '0';
            int ascii_sum = a + b + carry;
            
            // 两种方案都支持
#ifdef MAP
            result[k] = ASCII_INT_ADD_CARRY_MAP[ascii_sum];
            carry = (ascii_sum >= 106) ? 1 : 0;
#else
            result[k] = '0' + ((ascii_sum - 2 * '0') % 10);
            carry = (ascii_sum - 2 * '0') / 10;
#endif
            
            i--; j--; k--;
        }
        
        if (result[0] == '0') {
            return result.substr(1);
        }
        
        return result;
    }

public:
    std::string addStrings(std::string num1, std::string num2) {
        // 处理符号
        bool num1_negative = !num1.empty() && num1[0] == '-';
        bool num2_negative = !num2.empty() && num2[0] == '-';
        
        // 移除符号
        if (num1_negative) num1 = num1.substr(1);
        if (num2_negative) num2 = num2.substr(1);
        
        // 四种情况处理
        if (!num1_negative && !num2_negative) {
            // 正数 + 正数
            return addPositiveStrings(num1, num2);
        } else if (num1_negative && num2_negative) {
            // 负数 + 负数
            return "-" + addPositiveStrings(num1, num2);
        } else if (num1_negative && !num2_negative) {
            // 负数 + 正数 = 正数 - 负数绝对值
            if (isGreaterOrEqual(num2, num1)) {
                return subtractPositiveStrings(num2, num1);
            } else {
                return "-" + subtractPositiveStrings(num1, num2);
            }
        } else { // !num1_negative && num2_negative
            // 正数 + 负数 = 正数 - 负数绝对值
            if (isGreaterOrEqual(num1, num2)) {
                return subtractPositiveStrings(num1, num2);
            } else {
                return "-" + subtractPositiveStrings(num2, num1);
            }
        }
    }
};
