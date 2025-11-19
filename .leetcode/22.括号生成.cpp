/*
 * @lc app=leetcode.cn id=22 lang=cpp
 *
 * [22] 括号生成
 */

// @lc code=start
#include <vector>
#include <string>
using namespace std;

// 定义使用的解法
#define BACKTRACKING
//#define DYNAMIC_PROGRAMMING

class Solution {
public:
    vector<string> generateParenthesis(int n) {
#ifdef BACKTRACKING
        // 回溯法解决括号生成问题
        vector<string> result;  // 存储所有有效的括号组合
        string current;         // 当前正在构建的括号组合
        
        // 调用回溯函数生成所有组合
        backtrack(result, current, 0, 0, n);
        
        return result;
#elif defined(DYNAMIC_PROGRAMMING)
        // 动态规划法解决括号生成问题
        // dp[i]表示i对括号的所有有效组合
        vector<vector<string>> dp(n + 1);
        dp[0] = {""};  // 0对括号只有一种情况，即空字符串
        
        // 计算dp[1], dp[2], ..., dp[n]
        for (int i = 1; i <= n; i++) {
            // 对于i对括号，我们可以将其分解为j对括号和i-j-1对括号的组合
            for (int j = 0; j < i; j++) {
                // 遍历dp[j]中的每个组合
                for (const string& left : dp[j]) {
                    // 遍历dp[i-j-1]中的每个组合
                    for (const string& right : dp[i - j - 1]) {
                        // 将左边的组合、一对括号和右边的组合拼接起来
                        // 形式为: (left)right
                        dp[i].push_back("(" + left + ")" + right);
                    }
                }
            }
        }
        
        return dp[n];
#endif
    }

private:
#ifdef BACKTRACKING
    // 回溯函数
    // result: 存储结果的数组
    // current: 当前构建的字符串
    // openCount: 已使用的左括号数量
    // closeCount: 已使用的右括号数量
    // n: 括号对数
    void backtrack(vector<string>& result, string& current, int openCount, int closeCount, int n) {
        // 如果当前字符串长度等于2n，说明已经使用了n对括号，将其加入结果
        if (current.length() == n * 2) {
            result.push_back(current);
            return;
        }
        
        // 如果左括号数量小于n，可以添加左括号
        if (openCount < n) {
            current.push_back('(');
            backtrack(result, current, openCount + 1, closeCount, n);
            current.pop_back();  // 回溯，移除刚添加的左括号
        }
        
        // 如果右括号数量小于左括号数量，可以添加右括号
        if (closeCount < openCount) {
            current.push_back(')');
            backtrack(result, current, openCount, closeCount + 1, n);
            current.pop_back();  // 回溯，移除刚添加的右括号
        }
    }
#endif
};
// @lc code=end