/*
 * @lc app=leetcode.cn id=240 lang=cpp
 *
 * [240] 搜索二维矩阵 II
 */

// @lc code=start
#include <vector>
#include <ranges>
using namespace std;
class Solution {
public:
    /*
    1  4  7  11 15
    2  5  8  12 19
    3  6  9  21 22
    10 13 14 23 24
    */
    // 核心逻辑收缩
    // 0. 直接暴力：时间复杂度O(m*n)
    // 1. 左到右搜索：时间复杂度O(min(ROWS- row_shrink_v, COLS- col_shrink_v)^2 / 2)，取决于最小的一边，没有较好的利用题目说的行列有序条件
    // 2. 右到左搜索：时间复杂度O(m+n)，右上角开始，收缩列（值比目标大），扩大行（值比目标小）
    bool searchMatrix(vector<vector<int>>& matrix, int target) {
        if (matrix.empty() || matrix[0].empty()) return false;
        const int ROWS = matrix.size();
        const int COLS = matrix[0].size();
        if (target > matrix[ROWS - 1][COLS - 1]) return false;
        if (target == matrix[ROWS - 1][COLS - 1]) return true;
#ifdef LEFT_TO_RIGHT_SEARCH
        int row_shrink_v = 0, col_shrink_v = 0;
        int x = 0, y = 0, ncycles = min(ROWS, COLS);

        for (int n = 0; n < ncycles; ++n) {
            for(int row = y; row < ROWS - row_shrink_v; ++row) {
                if(matrix[row][y] == target) return true;
                else if(matrix[row][y] > target) { row_shrink_v = ROWS - row; break; }
            }
            for(int col = x + 1; col < COLS - col_shrink_v; ++col) {
                if(matrix[x][col] == target) return true;
                else if(matrix[x][col] > target) { col_shrink_v = COLS - col; break; }
            }
            ++x;
            ++y;
            ncycles = min(ROWS - row_shrink_v, COLS - col_shrink_v);
        }
#else
        // 从矩阵右上角开始搜索（该位置是行最大、列最小的元素）
        int ROW = 0, COL = COLS - 1;

        // 循环条件：行不越下界，列不越左界
        #pragma GCC unroll 16
        while (ROW != ROWS && COL != -1)
        {
            const int data = matrix[ROW][COL];
            if(data == target) return true;
            else if(data > target) --COL;// 当前值比目标大，向左移动一列（缩小列范围）
            else ++ROW; // 当前值比目标小，向下移动一行（扩大行范围）
        }
#endif
        return false;
    }
};
// @lc code=end
int main() {
    Solution s;
    vector<vector<int>> matrix = { {1,4,7,11,15},{2,5,8,12,19},{3,6,9,16,22},{10,13,14,17,24},{18,21,23,26,30} };
    int target = 5;
    s.searchMatrix(matrix, target);
    return 0;
}
