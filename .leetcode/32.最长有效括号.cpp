/*
 * @lc app=leetcode.cn id=32 lang=cpp
 *
 * [32] 最长有效括号
 */

// @lc code=start
#include <string>
#include <vector>
#include <algorithm>
#include <stack>
using namespace std;
//#define CURRENT_OPTIMIZED   // 当前解法优化版
//#define DP_SOLUTION         // 动态规划解法
#define STACK_SOLUTION      // 栈解法（最优）
/*
- 当前解法优化版 (CURRENT_OPTIMIZED)

- 简化了合并逻辑，使用排序后线性扫描的方式合并相邻括号对
- 减少了重复检查，提高了效率
- 时间复杂度仍为O(n²)，但常数因子更小
- 动态规划解法 (DP_SOLUTION)

- 定义dp[i]表示以s[i]结尾的最长有效括号长度
- 考虑两种情况：
  - s[i-1]s[i] = "()"形式
  - s[i]是")"且能找到匹配的"("
- 时间复杂度O(n)，空间复杂度O(n)
- 栈解法 (STACK_SOLUTION，最优)

- 使用栈记录可能的起始位置
- 遇到'('时入栈，遇到')'时出栈并计算当前有效长度
- 栈底保持一个特殊位置(-1)作为计算长度的参考点
- 时间复杂度O(n)，空间复杂度O(n)，但常数因子比动态规划小
*/
class Solution {
public:
// )()())((())())
// )0 2 4 5 9 10 12 13
// (1 3 6 7 8 11
// 先找多个最小()，然后每个()左右外扩，如果符合继续扩，不符合查看附近是否存在合理的括号，然后合并继续扩大，重复这个步骤
// 如 ()12 扩大是)()(0123不符合，但是邻近()34是符合的，合并()()1234得到长度4，继续扩大不符合
//最终可以得到((())())
    int longestValidParentheses(string s) {
        int n = s.length();
        if(n <= 1) return 0;
#ifdef CURRENT_OPTIMIZED
        // 记录每个位置是否是有效括号对的一部分
        vector<bool> valid(n, false);
        // 记录每个有效括号对的起始和结束位置
        vector<pair<int, int>> validPairs;
        
        // 第一步：找到所有基本的有效括号对 ()
        for(int i = 0; i < n - 1; i++) {
            if(s[i] == '(' && s[i+1] == ')') {
                valid[i] = valid[i+1] = true;
                validPairs.push_back({i, i+1});
            }
        }
        
        // 如果没有找到基本括号对，直接返回0
        if(validPairs.empty()) return 0;
        
        bool changed = true;
        // 不断尝试扩展和合并括号对，直到无法继续
        while(changed) {
            changed = false;
            vector<pair<int, int>> newPairs;
            
            // 先尝试扩展每个括号对
            for(int i = 0; i < validPairs.size(); i++) {
                int start = validPairs[i].first;
                int end = validPairs[i].second;
                bool expanded = false;
                
                // 尝试向左扩展
                if(start > 0 && end < n - 1 && s[start-1] == '(' && s[end+1] == ')' && !valid[start-1] && !valid[end+1]) {
                    valid[start-1] = valid[end+1] = true;
                    newPairs.push_back({start-1, end+1});
                    expanded = true;
                    changed = true;
                }
                
                // 如果没有扩展成功，保留原括号对
                if(!expanded) {
                    newPairs.push_back({start, end});
                }
            }
            
            // 更新有效括号对列表
            validPairs = newPairs;
             // 尝试合并相邻的括号对
            sort(validPairs.begin(), validPairs.end());
            newPairs.clear();
            
            for(int i = 0; i < validPairs.size(); i++) {
                int start = validPairs[i].first;
                int end = validPairs[i].second;
                
                // 向后合并所有可能的相邻括号对
                while(i + 1 < validPairs.size() && validPairs[i+1].first == end + 1) {
                    end = validPairs[i+1].second;
                    i++;
                    changed = true;
                }
                
                newPairs.push_back({start, end});
            }
            
            validPairs = newPairs;
        }
        
        int maxLength = 0;
        for(const auto& pair : validPairs) {
            maxLength = max(maxLength, pair.second - pair.first + 1);
        }
        
        return maxLength;
#elif defined(DP_SOLUTION)
// dp[i] 表示以索引i结尾的最长有效括号子串的长度
        vector<int> dp(n, 0);
        int maxLength = 0;
        
        // 从索引1开始，因为有效括号至少需要两个字符
        for (int i = 1; i < n; i++) {
            // 只有右括号')'才可能是有效括号子串的结尾
            if (s[i] == ')') {
                // 情况1：当前右括号与前一个字符组成"()"
                if (s[i-1] == '(') {
                    // 如果i>=2，需要加上i-2位置的有效长度；否则就是新的长度为2的有效子串
                    dp[i] = (i >= 2 ? dp[i-2] : 0) + 2;
                }
                // 情况2：当前右括号匹配更前面的左括号，形如"((...))"
                else if (i - dp[i-1] > 0 && s[i - dp[i-1] - 1] == '(') {
                    /*
                     * 解释：
                     * i - dp[i-1] - 1 是什么？
                     * dp[i-1] 是以s[i-1]结尾的有效括号长度
                     * 如果s[i-1]是')'且是有效括号的一部分，那么dp[i-1]>0
                     * i - dp[i-1] - 1 就是跳过这个有效括号子串，查看前面的字符
                     * 如果该位置是'('，则可以与当前的')'匹配
                     */
                    
                    // 当前匹配的括号对贡献2个长度，加上内部已经匹配的dp[i-1]
                    dp[i] = dp[i-1] + 2;
                    
                    // 还需要考虑新匹配的括号对之前是否还有有效括号
                    // 例如："()((...))"
                    //        ^    ^
                    //        |    |
                    //     i-dp[i-1]-2  i
                    if (i - dp[i-1] >= 2) {
                        dp[i] += dp[i - dp[i-1] - 2];
                    }
                }
                
                // 更新全局最大长度
                maxLength = max(maxLength, dp[i]);
            }
        }
        
        return maxLength;
#elif defined(STACK_SOLUTION)
        int maxLen = 0;
        stack<int> stk;
        stk.push(-1);  // 初始哨兵，便于计算长度
        
        for(int i = 0; i < n; i++) {
            if(s[i] == '(') {
                stk.push(i);
            } else { // s[i] == ')'
                stk.pop();
                if(stk.empty()) {
                    // 没有匹配的左括号，将当前位置作为新的起点
                    stk.push(i);
                } else {
                    // 计算当前有效括号长度
                    maxLen = max(maxLen, i - stk.top());
                }
            }
        }
        
        return maxLen;
#endif
    }
};
// @lc code=end