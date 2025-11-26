void setZeroes(vector<vector<int>>& matrix) {
        bool isFirstRowHasZero = false, isHasZero = false;
        auto& firstRowVec = matrix[0];
        int ROWS = matrix.size(), COLS = firstRowVec.size();

        for(int col = 0; col < COLS; ++col) {
            if (!firstRowVec[col]) {
                isFirstRowHasZero = true;
                break;
            }
        }

        for (int i = 1; i < ROWS; ++i) {
            auto& rowVec = matrix[i];
            for (int j = 0; j < COLS; ++j) {
                if (rowVec[j] == 0) {
                    firstRowVec[j] = 0;
                    isHasZero = true;
                }
            }
            if (isHasZero) {
                isHasZero = false;
                std::fill(rowVec.begin(), rowVec.end(), 0);
            }
        }

        for (int col = 0; col < COLS; ++col) {
            if (!firstRowVec[col]) {
                for (int row = 0; row < ROWS; ++row) {
                    matrix[row][col] = 0;
                }
            }
        }

        if (isFirstRowHasZero) std::fill(firstRowVec.begin(), firstRowVec.end(), 0);
    }

void setZeroes(vector<vector<int>>& matrix) {
        int ROWS = matrix.size(), COLS = matrix[0].size();
        std::bitset<200> bits;
        bool isRowHasZero = false;
        for(int row = 0; row < ROWS; ++row) {
            #pragma GCC unroll 8
            for(int col = 0; col < COLS; ++col) {
                if(matrix[row][col] == 0) {
                    bits[col] = 1;
                    isRowHasZero = true;
                }
            }
            if (isRowHasZero) {
                std::fill(matrix[row].begin(), matrix[row].end(), 0);
                isRowHasZero = false;
            }
        }
        #pragma GCC unroll 8
        for(int i = 0; i < 200; ++i) {
            if(bits[i] == 1) {
                for(int row = 0; row < ROWS; ++row) {
                    matrix[row][i] = 0;
                }
            }
        }
    }
class Solution {
public:
    void setZeroes(std::vector<std::vector<int>>& matrix) {
        int rowLength = matrix.size(), colLength = matrix.front().size();
        bool hasZero{ false };
        std::vector<uint8_t> zeros(colLength, 0);
        #pragma GCC unroll 4
        for(int row = 0; row != rowLength; ++row)
        {
            hasZero = false;
            #pragma GCC unroll 4
            for(int col = 0; col != colLength; ++col)
            {
                if(matrix[row][col] == 0) zeros[col] = 1, hasZero = true;
            }
            if (hasZero) std::fill(matrix[row].begin(), matrix[row].end(), 0);
        }
        #pragma GCC unroll 4
        for(int col = 0; col != colLength; ++col)
        {
            if(zeros[col] == 1) {
                #pragma GCC unroll 4
                for(int row = 0; row != rowLength; ++row)
                {
                    matrix[row][col] = 0;
                }
            }
        }
    }
};
/*
 * @lc app=leetcode.cn id=73 lang=cpp
 *
 * [73] 矩阵置零
 */

// @lc code=start
#include <vector>
#include <ranges>
#include <unordered_map>
using namespace std;
class Solution {
public:
// 1 3 1 5               1 0 0 0 
// 1 0 0 2  (1,1) (1,2)  0 0 0 0 
// 3 4 5 0  (3,3)        0 0 0 0 
// 4 2 3 4               4 0 0 0
// [1 3 1 5] [1 0 0 2] [3 4 5 0] [4 2 3 4]
// [1 0 0 0] [0 0 0 0] [0 0 0 0] [4 0 0 0]
// 时间2O(mn)；空间O(mn) 一个pair数组循环存储0的坐标，然后进行一次循环更新
// 时间2O(mn)；空间O(m + n) 两个bool数组，一次遍历存储行列是否为0，然后在进行一次遍历更新
// 时间O(m^2 * n)；空间O(n) 一个hash存储列是否已经处理0，通过这个判断是否跳过循环
// 如1 0 0 2，处理第一个0，循环列次（当前行跳过），将其他列数字更新为0，hash标记第1列（下标从0开始）已经处理，遍历完这一行，再次循环将这一行全部赋值0
// ①        |  ②           ③      |  ④           ⑤
// 1 0 1 5   |  1 0 0 5   |  1 0 0 5 |  1 0 0 0   |  1 0 0 0
// 1 0 0 2   |  1 0 0 2   |  0 0 0 0 |  1 0 0 0   |  0 0 0 0
// 3 0 5 0   |  3 0 0 0   |  3 0 0 0 |  3 0 0 0   |  0 0 0 0
// 4 0 3 4   |  4 0 0 4   |  4 0 0 4 |  4 0 0 0   |  4 0 0 0
// 
// 时间O(2m + 2n + 2(m-1)(n-1))；空间O(1)
/*
*   核心思想：复用矩阵的第一行和第一列作为标记位。
*   实现步骤：
*       1.预处理：检查并记录第一行和第一列是否原本含有0。
*       2.标记阶段：遍历内部元素，用第一行和第一列存储清零标记。
*       3.清零阶段：根据标记处理内部元素，最后处理首行首列。
*/
    void setZeroes(vector<vector<int>>&matrix) {
        // ================= 解法选择开关 =================
#define CONSTANT_SPACE  // 选择常量空间解法
//#define O_MN_SPACE      // 选择O(mn)空间解法
//#define O_M_PLUS_N_SPACE // 选择O(m+n)空间解法

#ifdef O_MN_SPACE
        // 方法1：O(mn)空间，直接复制矩阵标记
        if (matrix.empty()) return;
        vector<vector<bool>> flag(matrix.size(), vector<bool>(matrix[0].size(), false));

        // 第一次遍历记录所有0的位置
        for (int i = 0; i < matrix.size(); ++i) {
            for (int j = 0; j < matrix[0].size(); ++j) {
                if (matrix[i][j] == 0) flag[i][j] = true;
            }
        }

        // 第二次遍历根据标记置零
        for (int i = 0; i < matrix.size(); ++i) {
            for (int j = 0; j < matrix[0].size(); ++j) {
                if (flag[i][j]) {
                    // 清空整行
                    for (int k = 0; k < matrix[0].size(); ++k) matrix[i][k] = 0;
                    // 清空整列
                    for (int k = 0; k < matrix.size(); ++k) matrix[k][j] = 0;
                }
            }
        }

#elif defined(O_M_PLUS_N_SPACE)
        // 方法2：O(m+n)空间，行列标记数组
        if (matrix.empty()) return;
        vector<bool> rows(matrix.size(), false);
        vector<bool> cols(matrix[0].size(), false);

        // 记录需要清零的行列
        for (int i = 0; i < matrix.size(); ++i) {
            for (int j = 0; j < matrix[0].size(); ++j) {
                if (matrix[i][j] == 0) {
                    rows[i] = true;
                    cols[j] = true;
                }
            }
        }

        // 根据标记清零行
        for (int i = 0; i < rows.size(); ++i) {
            if (rows[i]) {
                for (int j = 0; j < matrix[0].size(); ++j) matrix[i][j] = 0;
            }
        }
        // 根据标记清零列
        for (int j = 0; j < cols.size(); ++j) {
            if (cols[j]) {
                for (int i = 0; i < matrix.size(); ++i) matrix[i][j] = 0;
            }
        }

#else
        // 方法3：常量空间，原地标记法
        if (matrix.empty()) return;
        bool first_row_has_zero = false;
        bool first_col_has_zero = false;
        const int ROWS = matrix.size();
        const int COLS = matrix[0].size();

        // 检查第一行是否有原始0
        for (int j = 0; j < COLS; ++j) {
            if (matrix[0][j] == 0) {
                first_row_has_zero = true;
                break;
            }
        }
        // 检查第一列是否有原始0
        for (int i = 0; i < ROWS; ++i) {
            if (matrix[i][0] == 0) {
                first_col_has_zero = true;
                break;
            }
        }

        // 使用第一行和第一列作为标记位
        for (int i = 1; i < ROWS; ++i) {
            for (int j = 1; j < COLS; ++j) {
                if (matrix[i][j] == 0) {
                    matrix[i][0] = 0; // 标记行头
                    matrix[0][j] = 0; // 标记列头
                }
            }
        }

        // 根据标记清零内部元素
        for (int i = 1; i < ROWS; ++i) {
            for (int j = 1; j < COLS; ++j) {
                if (matrix[i][0] == 0 || matrix[0][j] == 0) {
                    matrix[i][j] = 0;
                }
            }
        }

        // 处理第一行
        if (first_row_has_zero) {
            for (int j = 0; j < COLS; ++j) matrix[0][j] = 0;
        }
        // 处理第一列
        if (first_col_has_zero) {
            for (int i = 0; i < ROWS; ++i) matrix[i][0] = 0;
        }
#endif
    }
};
// @lc code=end

