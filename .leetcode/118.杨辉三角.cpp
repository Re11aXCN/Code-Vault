/*
 * @lc app=leetcode.cn id=118 lang=cpp
 *
 * [118] 杨辉三角
 */

// @lc code=start
#include <vector>
using namespace std;

class Solution {
public:
    vector<vector<int>> generate(int numRows) {
        std::vector<std::vector<int>> result; result.reserve(numRows);
        #pragma GCC unroll 4
        for (int i = 0; i < numRows; ++i) {
            // 每行的元素个数等于行号+1
            result.emplace_back(i + 1, 1); // 初始化为1，因为每行的首尾都是1

            // 计算中间的元素（第一行和第二行不需要计算）
            #pragma GCC unroll 4
            for(int j = 1; j < i; ++j) {
                // 当前元素等于上一行的相邻两个元素之和
                result[i][j] = result[i-1][j-1] + result[i-1][j];
            }
        }
        return result;
    }
};
// @lc code=end