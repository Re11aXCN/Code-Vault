/*
 * @lc app=leetcode.cn id=459 lang=cpp
 *
 * [459] 重复的子字符串
 */

// @lc code=start
class Solution {
public:
    // 利用错位相等的特性，KMP算法的next符合这个特性
    // 求最长相等前后缀字符串长度，如果这个长度能够被 主串整除 说明找到
    bool repeatedSubstringPattern(const std::string& s) {
        size_t str_len = s.size();
        // 创建next数组（前缀表），初始化为0，长度与字符串相同
        // next[i]表示：以s[i]结尾的子串中，最长相同前后缀的长度
        std::vector<int> next(str_len, 0);
        
        // 构造KMP算法的next数组
        // i：当前正在计算next值的位置（后缀末尾）
        // j：已匹配的前缀长度，也指向前缀的下一个字符位置
        for (int i = 1, j = 0; i < str_len; ++i) {
           // 当字符不匹配且j>0时，回退到前一个字符对应的next值位置
           // 利用已计算的部分匹配信息，避免从头开始匹配
           while (j > 0 && s[i] != s[j]) j = next[j - 1];

           // 如果当前字符匹配，前缀长度+1
           if (s[i] == s[j]) ++j;
           
           // 记录当前位置的next值（最长相同前后缀长度）
           next[i] = j; 
        }

// 判断字符串是否由重复子串构成：
// 1. next.back() > 0：最后一位存在相同前后缀
// 2. str_len % (str_len - next.back()) == 0：
//    - (str_len - next.back()) 得到最小重复单元的长度
//    - 如果字符串长度能被这个长度整除，说明整个字符串由该单元重复构成
        return next.back() > 0 && str_len % (str_len - next.back()) == 0;
    }
};
bool repeatedSubstringPattern(const std::string& s) {
    std::string str{ s + s };
    return std::search(std::next(str.begin()), std::prev(str.end()), s.begin(), s.end())
        != std::prev(str.end());
}
// @lc code=end

