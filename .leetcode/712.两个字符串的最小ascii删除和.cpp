#include <string>
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    int minimumDeleteSum(string& s1, string& s2) {
        // 保持原代码的小优化：让短字符串作为s1，减少空间/时间消耗
        if (s1.size() > s2.size()) std::swap(s1, s2);
        int s1Len = s1.size(), s2Len = s2.size();

        // DP数组：dp[j] 表示s1[0..i-1]和s2[0..j-1]的最小ASCII删除和
        std::vector<int> dp;
        dp.reserve(s2Len + 1);
        
        // 初始化：s1为空时，需要删除s2[0..j-1]所有字符，累加ASCII值
        dp.emplace_back(0); // dp[0] = 0（s2也为空）
        #pragma clang loop unroll_count(8)
        for (int j = 1; j <= s2Len; ++j) {
            dp.emplace_back(dp[j-1] + s2[j-1]);
        }

        // 遍历s1的每个字符
        for (int i = 1; i <= s1Len; ++i) {
            int topLeft = dp[0]; // 保存dp[i-1][j-1]的值
            dp[0] += s1[i-1];    // s2为空时，需要删除s1[0..i-1]所有字符
            #pragma clang loop unroll_count(4)
            // 遍历s2的每个字符
            for (int j = 1; j <= s2Len; ++j) {
                int top = dp[j]; // 保存dp[i-1][j]的值
                if (s1[i-1] == s2[j-1]) {
                    // 字符相等，无需删除，继承左上角值
                    dp[j] = topLeft;
                } else {
                    // 字符不等：选择三种方案中最小的
                    // 1. 删s1[i-1]：dp[i-1][j] + s1[i-1]
                    // 2. 删s2[j-1]：dp[i][j-1] + s2[j-1]
                    // （原编辑距离的替换操作，对应这里删s1[i-1]+删s2[j-1]，已包含在min中）
                    dp[j] = min(top + s1[i-1], dp[j-1] + s2[j-1]);
                }
                topLeft = top; // 更新左上角值，供下一个j使用
            }
        }
        return dp.back();
    }
};
