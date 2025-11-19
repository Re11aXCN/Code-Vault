
/*
 * @lc app=leetcode.cn id=74 lang=cpp
 *
 * [74] 搜索二维矩阵
 */

// @lc code=start
#include <algorithm>
class Solution {
public:
    // O(log(m*n))
    bool searchMatrix(std::vector<std::vector<int>>& matrix, int target) {
        int ROWS = matrix.size(), COLS = matrix.front().size();
        int left{0}, right{ROWS * COLS - 1};
        /*
        一个二维数组视为一维的 vec[idx] ==vec[row * COLS + col] == vec[row][col]
        row * COLS + col = idx
        row = (idx- col) / COLS; // 恰好被整除
        col = idx- row * COLS;
        公式转化，(idx - col) / COLS 等于 idx / COLS - col / COLS
        如果我们只关心行，根据计算机计算，除法不被整除，余数会被省略，所以row可以等价于 idx / COLS，
        那么余数是 col / COLS ，col 可能被整除可能不被整除，等价于 idx % COLS
        */
        while(left <= right)
        {
            int mid = left + ((right - left) >> 1);
            auto [row, col] = std::div(mid, COLS);
            /*
            int row = mid / COLS;   // 计算行索引
            int col = mid % COLS;   // 计算列索引
            */
            if(matrix[row][col] < target) left = left + 1;
            else if(matrix[row][col] > target) right = right - 1;
            else return true;
        }
        return false;
    }

    // O(m*log(n))
    bool searchMatrix(vector<vector<int>>& matrix, int target) {
        for(int i = 0; i < matrix.size(); ++i){
            if(matrix[i].front() <= target && target <= matrix[i].back()){
                return std::binary_search(matrix[i].begin(), matrix[i].end(), target);
            }
        }
        return false;
    }

    // O(m*n)
    bool searchMatrix(vector<vector<int>>& matrix, int target) {
        for(int row = 0, col = matrix[0].size() - 1; row < matrix.size(); ++row) {
            for( ; col >= 0; --col) {
                if(matrix[row][col] == target) return true;
                else if (matrix[row][col] < target) break;
            }
            if(col < 0) return false;
        }
        return false;
    }
};
// @lc code=end

