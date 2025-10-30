#include <iostream>
#include <vector>
#include <chrono>
#include <random>

// 版本1: 直接旋转（原始版本）
static void rotateMatrixDirect(std::vector<std::vector<int>>& matrix) {
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

// 版本2: 转置 + 逆序
static void rotateMatrixTranspose(std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();
    // 转置
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            std::swap(matrix[i][j], matrix[j][i]);
        }
    }
    // 每行逆序
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n / 2; ++j) {
            std::swap(matrix[i][j], matrix[i][n - 1 - j]);
        }
    }
}

// 版本3: 直接旋转 + 循环展开
static void rotateMatrixDirectUnrolled(std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();
    for (int i = 0; i < n / 2; ++i) {
        int j = i;
        // 主循环：每次处理2个元素
        for (; j < n - i - 2; j += 2) {
            // 第一个元素旋转
            int temp1 = matrix[i][j];
            matrix[i][j] = matrix[n - 1 - j][i];
            matrix[n - 1 - j][i] = matrix[n - 1 - i][n - 1 - j];
            matrix[n - 1 - i][n - 1 - j] = matrix[j][n - 1 - i];
            matrix[j][n - 1 - i] = temp1;

            // 第二个元素旋转
            int temp2 = matrix[i][j + 1];
            matrix[i][j + 1] = matrix[n - 2 - j][i];
            matrix[n - 2 - j][i] = matrix[n - 1 - i][n - 2 - j];
            matrix[n - 1 - i][n - 2 - j] = matrix[j + 1][n - 1 - i];
            matrix[j + 1][n - 1 - i] = temp2;
        }
        // 处理剩余元素（0或1个）
        for (; j < n - i - 1; ++j) {
            int temp = matrix[i][j];
            matrix[i][j] = matrix[n - 1 - j][i];
            matrix[n - 1 - j][i] = matrix[n - 1 - i][n - 1 - j];
            matrix[n - 1 - i][n - 1 - j] = matrix[j][n - 1 - i];
            matrix[j][n - 1 - i] = temp;
        }
    }
}

// 版本4: 转置 + 逆序 + 循环展开
static void rotateMatrixTransposeUnrolled(std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();

    // 转置 + 循环展开
    for (int i = 0; i < n; ++i) {
        int j = i + 1;
        // 主循环：每次处理2个元素
        for (; j < n - 1; j += 2) {
            std::swap(matrix[i][j], matrix[j][i]);
            std::swap(matrix[i][j + 1], matrix[j + 1][i]);
        }
        // 处理剩余元素（0或1个）
        for (; j < n; ++j) {
            std::swap(matrix[i][j], matrix[j][i]);
        }
    }

    // 逆序 + 循环展开
    for (int i = 0; i < n; ++i) {
        int j = 0;
        // 主循环：每次处理2个元素
        for (; j < n / 2 - 1; j += 2) {
            std::swap(matrix[i][j], matrix[i][n - 1 - j]);
            std::swap(matrix[i][j + 1], matrix[i][n - 2 - j]);
        }
        // 处理剩余元素
        for (; j < n / 2; ++j) {
            std::swap(matrix[i][j], matrix[i][n - 1 - j]);
        }
    }
}

class MatrixRotateBenchmark {
private:
    std::vector<int> sizes;
    std::mt19937 rng;

public:
    MatrixRotateBenchmark() : rng(std::random_device{}()) {
        // 测试不同大小的矩阵，包括奇数和偶数
        sizes = { 5, 20, 128, 129, 256, 257, 512, 513, 1024, 1025 };
    }

    // 创建随机矩阵
    std::vector<std::vector<int>> createRandomMatrix(int n) {
        std::vector<std::vector<int>> matrix(n, std::vector<int>(n));
        std::uniform_int_distribution<int> dist(0, 1000);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                matrix[i][j] = dist(rng);
            }
        }
        return matrix;
    }

    // 验证矩阵是否正确旋转
    bool verifyRotation(const std::vector<std::vector<int>>& original,
        const std::vector<std::vector<int>>& rotated) {
        int n = original.size();
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (rotated[i][j] != original[n - 1 - j][i]) {
                    return false;
                }
            }
        }
        return true;
    }

    // 复制矩阵
    std::vector<std::vector<int>> copyMatrix(const std::vector<std::vector<int>>& matrix) {
        return matrix;
    }

    // 运行性能测试
    void runBenchmark() {
        std::cout << "矩阵旋转性能测试 (循环2次)\n";
        std::cout << "========================================\n";

        for (int n : sizes) {
            std::cout << "\n矩阵大小: " << n << "x" << n << "\n";
            std::cout << "----------------------------------------\n";

            auto original = createRandomMatrix(n);

            // 测试直接旋转法
            auto matrix1 = copyMatrix(original);
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < 2; ++i) rotateMatrixDirect(matrix1);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            bool valid1 = verifyRotation(original, matrix1);

            // 测试转置+逆序法
            auto matrix2 = copyMatrix(original);
            start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < 2; ++i) rotateMatrixTranspose(matrix2);
            end = std::chrono::high_resolution_clock::now();
            auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            bool valid2 = verifyRotation(original, matrix2);

            // 测试直接旋转+循环展开
            auto matrix3 = copyMatrix(original);
            start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < 2; ++i) rotateMatrixDirectUnrolled(matrix3);
            end = std::chrono::high_resolution_clock::now();
            auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            bool valid3 = verifyRotation(original, matrix3);

            // 测试转置+逆序+循环展开
            auto matrix4 = copyMatrix(original);
            start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < 2; ++i) rotateMatrixTransposeUnrolled(matrix4);
            end = std::chrono::high_resolution_clock::now();
            auto duration4 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            bool valid4 = verifyRotation(original, matrix4);

            std::cout << "直接旋转法: " << duration1.count() << " μs, 正确性: "
                << (valid1 ? "✓" : "✗") << "\n";
            std::cout << "转置+逆序法: " << duration2.count() << " μs, 正确性: "
                << (valid2 ? "✓" : "✗") << "\n";
            std::cout << "直接旋转(循环展开): " << duration3.count() << " μs, 正确性: "
                << (valid3 ? "✓" : "✗") << "\n";
            std::cout << "转置+逆序(循环展开): " << duration4.count() << " μs, 正确性: "
                << (valid4 ? "✓" : "✗") << "\n";
        }
    }
};

/*
int main() {
    // 简单正确性测试
    std::vector<std::vector<int>> testMatrix = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    std::cout << "原始矩阵 (3x3 奇数):\n";
    for (const auto& row : testMatrix) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }

    auto rotated = testMatrix;
    rotateMatrixDirect(rotated);

    std::cout << "\n旋转后矩阵:\n";
    for (const auto& row : rotated) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }

    // 运行性能测试
    MatrixRotateBenchmark benchmark;
    benchmark.runBenchmark();

    return 0;
}
*/

// 直接旋转 优于 转置+逆序，
// 循环展开2次 小概率比 优于不展开的