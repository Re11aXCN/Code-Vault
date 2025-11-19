class Solution {
public:
    void rotate(std::vector<std::vector<int>>& matrix) {
        int n = matrix.size();
        for (int i = 0; i != n >> 1; ++i) {
            const auto& end = n - i - 1;
            for (int j = i; j != end; ++j) {
                int temp = matrix[i][j];
                auto& _1 = matrix[n - 1 - j][i];
                auto& _2 = matrix[end][n - 1 - j];
                auto& _3 = matrix[j][end];
                matrix[i][j] = _1;
                _1 = _2;
                _2 = _3;
                _3 = temp;
            }
        }
    }
};

/*
 * @lc app=leetcode.cn id=48 lang=cpp
 *
 * [48] 旋转图像
 */

// @lc code=start
#include <vector>
#include <algorithm> // 用于reverse函数
using namespace std;
class Solution {
public:
// 1  2  3  4
// 5  6  7  8
// 9  10 11 12
// 13 14 15 16

// 13 9  5 1
// 14 10 6 2
// 15 11 7 3
// 16 12 8 4
//                                            15换9             9换8             9换2
//  13 2  3  4  | 13  2  3  4  |  13  2  3  1   |  13  2  3  1   |  13  2  3  1   |  13  9  3  1  |
//  5  6  7  8  | 5   6  7  8  |  5   6  7  8   |  5   6  7  8   |  5   6  7  9   |  5   6  7  2  |
//  9  10 11 12 | 9   10 11 12 |  9   10 11 12  |  15  10 11 12  |  15  10 11 12  |  15  10 11 12 |
//  1  14 15 16 | 16  14 15 1  |  16  14 15 4   |  16  14 9  4   |  16  14 8  4   |  16  14 8  4  |
//
//14换5          5换12        5换3  
//  13  9  3  1  |  13  9  3  1  |  13  9  5  1 |                            |  13 9  5 1 |
//  14  6  7  2  |  14  6  7  2  |  14  6  7  2 |  10 换 6 -> 6换11 -> 6 换7 |  14 10 6 2 |
//  15  10 11 12 |  15  10 11 5  |  15  10 11 3 |                            |  15 11 7 3 |
//  16  5  8  4  |  16  12 8  4  |  16  12 8  4 |                            |  16 12 8 4 |
// 本质：第一列变为第一行，第二列变为第二行，以此类推
// 实际代码，当前仅当矩阵n*n的n>=2的时候，每个数交换3次就能够得到正确的位置，如上述的14换5、5换12、5换3，如10换6、6换11、6换7
// 如果是1不用交换
// 递归或者迭代，每次只处理一圈，由外向内，圈需要循环的次数n，n-2，n-4以此类推
// 第一层：
// 先处理一行1 2 3 4，注意题目要求顺时针，1交换13  1交换16 1交换4  2交换9  2交换15  2交换8 ....
// 矩阵逻辑：[0][0]交换[3][0] [3][0]交换[3][3] [3][3]交换[0][3]，[0][1]交换[2][0] [2][0]交换[3][2] [3][2]交换[1][3]
// 第一层         上左，左下，下右，[0][i] 和 [n-i][0]  [n-i][0]和[3][n-i]  [3][n-i]和[i][3]    
// 第二层         上左，左下，下右，[1][i] 和 [n-i][1]  [n-i][1]和[2][n-i]  [2][n-i]和[i][2] 
// 实际公式[层数-1][i] 和 [n-i][层数-1]  [n-i][层数-1]和[n-层数+1][n-i]  [n-层数+1][n-i]和[i][n-层数+1] 
    void rotate(vector<vector<int>>& matrix) {
        // ================ 方法选择开关 ================
#define TRANSPOSE_AND_REVERSE  // 转置后反转每行
//#define LAYER_ROTATION        // 分层逐层旋转

#ifdef TRANSPOSE_AND_REVERSE
        /* 方法一：转置矩阵后反转每行*/
        //数学原理
        // 转置：将矩阵沿主对角线镜像对称，行列索引互换。
        // 反转行：将每行元素逆序排列，相当于垂直镜像对称。
        const int n = matrix.size();

        // 1. 转置矩阵：交换matrix[i][j]和matrix[j][i] (i < j)
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) { // 注意j从i+1开始避免重复交换
                swap(matrix[i][j], matrix[j][i]);
            }
        }

        // 2. 反转每行元素
        for (int i = 0; i < n; ++i) {
            reverse(matrix[i].begin(), matrix[i].end());
        }

#else
        /* 方法二：分层逐层旋转 */
        const int n = matrix.size();
        // 分层处理，总共有n/2层
        for (int layer = 0; layer < n / 2; ++layer) {
            // 每层处理的起始和结束索引
            const int start = layer;
            const int end = n - 1 - layer;

            // 遍历当前层的每一组元素（每层有end - start组）
            for (int i = start; i < end; ++i) {
                const int offset = i - start; // 当前组在层内的偏移量
                // 保存左上角元素
                int temp = matrix[start][i];
                // 左下→左上
                matrix[start][i] = matrix[end - offset][start];
                // 右下→左下
                matrix[end - offset][start] = matrix[end][end - offset];
                // 右上→右下
                matrix[end][end - offset] = matrix[i][end];
                // 临时值→右上
                matrix[i][end] = temp;
            }
        }
#endif
    }
};
// @lc code=end

