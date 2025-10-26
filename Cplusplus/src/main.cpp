#include "header.h"
#include <boost/container/small_vector.hpp>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <tbb/parallel_for.h>
#include <tbb/parallel_invoke.h>
#include <tbb/parallel_sort.h>
#include <tbb/concurrent_vector.h>
#include <tbb/enumerable_thread_specific.h>
#include <immintrin.h>

#include "parallel/filter_sin_greater_than_0.h"
#include "sort/benchmark_sort.hpp"
using std::string;
using std::vector;
using std::queue;
using namespace std::literals;
#include <vector>

void matrix_multiply_accumulate(std::vector<std::vector<double>>& a,
    const std::vector<std::vector<double>>& b,
    const std::vector<std::vector<double>>& c) {
    int m = b.size();    // b 的行数
    int n = c[0].size(); // c 的列数
    int p = c.size();    // c 的行数（也是 b 的列数）

    // 检查矩阵维度是否匹配
    if (b[0].size() != p || a.size() != m || a[0].size() != n) {
        throw std::invalid_argument("Matrix dimensions do not match for multiplication");
    }

    // 临时存储 b * c 的结果
    std::vector<std::vector<double>> temp(m, std::vector<double>(n, 0.0));

    // 计算矩阵乘法 b * c
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < p; ++k) {
                temp[i][j] += b[i][k] * c[k][j];
            }
        }
    }

    // 将结果累加到 a
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            a[i][j] += temp[i][j];
        }
    }
}
int main() {
    std::cout.setf(std::ios_base::boolalpha);
    filter_sin_greater_than_zero();
    benchmark_sort();
    //// 初始化矩阵
    //std::vector<std::vector<double>> a = { {1, 2}, {3, 4} };
    //std::vector<std::vector<double>> b = { {1, 0}, {0, 1} };
    //std::vector<std::vector<double>> c = { {5, 6}, {7, 8} };
    //TICK(matrix_multiply);
    //// 执行 a += b * c
    //matrix_multiply_accumulate(a, b, c);
    //TOCK(matrix_multiply);
    return 0;
}
