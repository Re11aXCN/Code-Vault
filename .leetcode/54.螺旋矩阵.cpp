/*
 * @lc app=leetcode.cn id=54 lang=cpp
 *
 * [54] 螺旋矩阵
 */

// @lc code=start


    vector<int> spiralOrder(vector<vector<int>>& matrix) {
        if (matrix.empty() || matrix[0].empty()) return {};
        
        int row = matrix.size();
        int col = matrix.front().size();
        vector<int> res(row * col, 0);
        auto top_row = matrix.begin(), bottom_row = matrix.end() - 1;
        int round = row >> 1;
        auto it = res.begin();
        for (int i = 0; i < round; ++i, ++top_row, --bottom_row)
        {
            const int diff = (*top_row).size() - i - i;
            // 顶左右
            std::copy((*top_row).begin() + i, (*top_row).end() - i, it);
            it += diff;

            // 上到下
            for (auto r = top_row + 1; r != bottom_row; ++r) {
                *it = *((*r).end() - 1 - i);
                ++it;
            }

            // 底右左
            std::copy((*bottom_row).rbegin() + i, (*bottom_row).rend() - i, it);
            it += diff;

            // 下到上
            if (it == res.end()) return res; // 处理col == 1
            for (auto r = bottom_row - 1; r != top_row; --r) {
                *it = *((*r).begin() + i);
                ++it;
            }
            if (it == res.end()) return res; // 处理col == 2
        }

        // 奇数 剩余 中心部分
        if(row & 1) {
            std::copy((*top_row).begin() + round, (*top_row).end() - round, it);
        }
        return res;
    }
#include <vector>
using namespace std;
class Solution {
public:
// 1  2  3  4
// 5  6  7  8
// 9  10 11 12
// 13 14 15 16
// 1 2 3 4 8 12 16 15 14 13 9 5 6 7 11 10
// row * col - (row - 2) * 2
// 顺序，  end + col，倒序，       begin - col
// 1234   8 12 16     15 14 13    9 5
// 67     11          10
// 四次不同的for处理四个部分，递归/迭代一圈一圈的处理？
    vector<int> spiralOrder(vector<vector<int>>& matrix) {
        vector<int> result;
        if (matrix.empty()) return result;

        // ================= 解法选择开关 =================
#define ITERATIVE_METHOD   // 选择迭代法（标准最优解）
//#define RECURSIVE_METHOD  // 选择递归法

#ifdef ITERATIVE_METHOD
        // ================ 标准迭代解法（最优）================
        const int ROWS = matrix.size();
        const int COLS = matrix[0].size();
        int top = 0, bottom = ROWS - 1;
        int left = 0, right = COLS - 1;
        result.reserve(ROWS * COLS);
        while (top <= bottom && left <= right) {
            // 1. 遍历顶层：左→右
            for (int j = left; j <= right; ++j) {
                result.push_back(matrix[top][j]);
            }
            if (++top > bottom) break; // 更新上边界并检查是否越界

            // 2. 遍历右侧：上→下
            for (int i = top; i <= bottom; ++i) {
                result.push_back(matrix[i][right]);
            }
            if (--right < left) break; // 更新右边界并检查

            // 3. 遍历底层：右→左（确保存在多行）
            if (top <= bottom) {
                for (int j = right; j >= left; --j) {
                    result.push_back(matrix[bottom][j]);
                }
                --bottom; // 更新下边界
            }

            // 4. 遍历左侧：下→上（确保存在多列）
            if (left <= right) {
                for (int i = bottom; i >= top; --i) {
                    result.push_back(matrix[i][left]);
                }
                ++left; // 更新左边界
            }
        }

#elif defined(RECURSIVE_METHOD)
        // ================ 递归解法 ================
        function<void(int, int, int, int)> spiralLayer;
        spiralLayer = [&](int t, int b, int l, int r) {
            if (t > b || l > r) return;

            // 1. 左→右遍历顶层
            for (int j = l; j <= r; ++j) result.push_back(matrix[t][j]);
            if (++t > b) return;

            // 2. 上→下遍历右侧
            for (int i = t; i <= b; ++i) result.push_back(matrix[i][r]);
            if (--r < l) return;

            // 3. 右→左遍历底层（需存在多行）
            if (t <= b) {
                for (int j = r; j >= l; --j) result.push_back(matrix[b][j]);
                --b;
            }

            // 4. 下→上遍历左侧（需存在多列）
            if (l <= r) {
                for (int i = b; i >= t; --i) result.push_back(matrix[i][l]);
                ++l;
            }

            spiralLayer(t, b, l, r); // 递归处理内层
            };

        spiralLayer(0, matrix.size() - 1, 0, matrix[0].size() - 1);
#endif

        return result;
    }
};
// @lc code=end

int main() {
    vector<vector<int>> matrix =
    /*
    {
        { 1, 2, 3, 4, 5, 6, 7 },
        { 8, 9, 10, 11, 12, 13, 14 },
        { 15, 16, 17, 18, 19, 20, 21 },
        { 22, 23, 24, 25, 26, 27, 28 },
        { 29, 30, 31, 32, 33, 34, 35 },
        { 36, 37, 38, 39, 40, 41, 42 },
        //{ 43, 44, 45, 46, 47, 48, 49 }
    };
    */
    /*{
        {1, 2, 3,4,5,6},
        {7, 8, 9,10,11,12},
        {13,14,15,16,17,18},
        {19,20,21,22,23,24},
        {25,26,27,28,29,30},
        //{31,32,33,34,35,36}
    };*/
//{ {1}, {2}, {3} };
    //[[1,11],[2,12],[3,13],[4,14],[5,15],[6,16],[7,17],[8,18],[9,19],[10,20]]
{
    {1,11},
    {2,12},
    {3,13},
    {4,14},
    {5,15},
    {6,16},
    {7,17},
    {8,18},
    {9,19},
    {10,20}
};
    auto res = spiralOrder(matrix);
    for (int i = 0; i < res.size(); ++i) {
        std::cout << res[i] << " ";
    }
}