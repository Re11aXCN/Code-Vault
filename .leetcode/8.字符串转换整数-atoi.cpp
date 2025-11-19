class Solution {
public:
    int myAtoi(std::string s) {
        if (s.empty()) return 0;

        int i = 0;
        for (; i < s.length() && std::isspace(s[i]); ++i);
        if (i == s.length()) return 0; // 全是空格
        
        enum Flag {NoFlag = 0, Negative, Positive};
        Flag flag = NoFlag;
        if (s[i] == '-') {flag = Negative; ++i;}
        else if (s[i] == '+') {flag = Positive;++i;}

        if (i == s.length() || !std::isdigit(s[i])) return 0;

        int res = 0;
        if (flag == Negative) { // 对于负数：res * 10 - digit >= min_limit
            for (; i < s.length(); ++i) {
                if (!std::isdigit(s[i])) break;
                int digit = s[i] - '0';
                if (res < INT_MIN / 10 || (res == INT_MIN / 10 && digit > 8)) {
                    return INT_MIN;
                }
                res = res * 10 - digit;
            }
        }
        else { // 对于正数：res * 10 + digit <= max_limit
            for (; i < s.length(); ++i) {
                if (!std::isdigit(s[i])) break;
                int digit = s[i] - '0';
                if (res > INT_MAX / 10 || (res == INT_MAX / 10 && digit > 7)) {
                    return INT_MAX;
                }
                res = res * 10 + digit;
            }
        }
        return res;
    }
};
/*
 * @lc app=leetcode.cn id=8 lang=cpp
 *
 * [8] 字符串转换整数 (atoi)
 */

// @lc code=start
#include <climits>
#include <cctype>
#include <string>
using namespace std;
class Solution {
public:
    template<typename T>
    T atoi_impl(const char* str)
    {
        while (std::isspace(static_cast<unsigned char>(*str)))
            ++str;   
        bool negative = false;
 
        if (*str == '+')
            ++str;
        else if (*str == '-')
        {
            ++str;
            negative = true;
        }
        T result = 0;
        for (; std::isdigit(static_cast<unsigned char>(*str)); ++str)
        {
            int digit = *str - '0';
            result *= 10;
            result -= digit; // 计算负值，以支持 INT_MIN, LONG_MIN,..
            if (result <= INT_MIN)
                return negative ? INT_MIN : INT_MAX;
        }
    
        return negative ? result : -result;
    }
    int myAtoi(string s) {
        return atoi_impl<long>(s.c_str());
    }
};
/* 条件不够全
class Solution {
public:
    int myAtoi(std::string s) {
        if (s.empty() || std::isalpha(s.front()) || s.front() == '.') return 0;

        int i = 0;
        for (; i < s.length() && std::isspace(s[i]); ++i);
        if (i + 1 == s.length() || s[i] == '.') return 0;

        enum Flag {NoFlag = 0, Negative, Positive};
        Flag flag = s[i] == '-' ? Negative
            : s[i] == '+' ? Positive
            : NoFlag;
        if (flag != NoFlag && i + 1 == s.length()) return 0;
        if (!std::isdigit(s[i + 1])) return flag == NoFlag ? s[i] - '0' : 0;

        if (flag != NoFlag) ++i;
        int res = 0;//s[flag == NoFlag ? i++ : ++i] - '0';

        for (; i < s.length(); ++i)
        {
            if (!std::isdigit(s[i])) break;
            int digit = s[i] - '0';

            if (flag == Negative) {
                if (res < INT_MIN / 10 || (res == INT_MIN / 10 && digit > 8)) {
                    return INT_MIN;
                }
                res = res * 10 - digit;
            }
            else {
                if (res > INT_MAX / 10 || (res == INT_MAX / 10 && digit > 7)) {
                    return INT_MAX;
                }
                res = res * 10 + digit;
            }
        }
        return res;
    }
};*/
// @lc code=end
int main() {
    Solution s;
    string str = "20000000000000000000";
    int ans = s.myAtoi(str);
    return 0;
}

