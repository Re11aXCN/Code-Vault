#include <vector>
#include <algorithm>
#include <iterator>
/*
 * @lc app=leetcode.cn id=63 lang=cpp
 *
 * [63] 不同路径 II
 */

// @lc code=start
class Solution {
public:
    int uniquePathsWithObstacles(std::vector<std::vector<int>>& obstacleGrid) {
        auto& firstRowVec = obstacleGrid.front();
        std::size_t ROWS{ obstacleGrid.size() }, COLS{ firstRowVec.size() };
        // 如果起点或终点有障碍物，直接返回0
        if (obstacleGrid[0][0] == 1 || obstacleGrid[ROWS - 1][COLS - 1] == 1) {
            return 0;
        }
        std::vector<int> dp;    dp.reserve(COLS);
        dp.emplace_back(1);
        std::size_t idx = 1;
        #pragma GCC unroll 8
        for(auto it = std::next(firstRowVec.begin()); it != firstRowVec.end(); ++it, ++idx) {
            dp.emplace_back(*it == 1 ? 0 : dp[idx - 1]); // 有障碍说明无法到达 设置为0，否则是左边值能够到达当前
        }

        for (std::size_t i = 1; i < ROWS; ++i) {
            if (obstacleGrid[i].front() == 1) dp.front() = 0;
            #pragma GCC unroll 8
            for (std::size_t j = 1; j < COLS; ++j) {
                if(obstacleGrid[i][j] == 1) dp[j] = 0; // 有障碍说明无法到达 设置为0
                else dp[j] += dp[j - 1]; // curr = top + left 
            }
        }
        return dp.back();
    }
};
// @lc code=end

