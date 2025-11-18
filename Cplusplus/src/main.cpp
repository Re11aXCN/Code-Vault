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
#include <experimental/generator>

#include <tbb/parallel_for.h>
#include <tbb/parallel_invoke.h>
#include <tbb/parallel_sort.h>
#include <tbb/concurrent_vector.h>
#include <tbb/enumerable_thread_specific.h>
#include <immintrin.h>

#include "parallel/filter_sin_greater_than_0.h"
#include "sort/benchmark_parallel_sort.hpp"
#include "sort/radix_sort.hpp"
#include "parallel/avx_filter.hpp"
#include "leetcode/0048_rotate_image.h"
#include "leetcode/0912_sort_array.h"
using std::string;
using std::vector;
using std::queue;
using namespace std::literals;
template<typename Ty>
struct Matrix2D {
    static_assert(std::is_integral_v<Ty>&& std::is_unsigned_v<Ty>, "Ty must be an unsigned integral type.");

    std::array<Ty, 4> data;
    constexpr Matrix2D() : data{} {}
    constexpr Matrix2D(Ty a, Ty b, Ty c, Ty d)
        : data{ a, b, c, d } {
    }
    ~Matrix2D() = default;

    constexpr Matrix2D(const Matrix2D& other) = default;
    constexpr Matrix2D(Matrix2D&& other) noexcept = default;
    constexpr Matrix2D& operator=(const Matrix2D& other) = default;
    constexpr Matrix2D& operator=(Matrix2D&& other) noexcept = default;

    constexpr Matrix2D& operator*=(const Matrix2D& other) noexcept {
        auto [a, b, c, d] = data;
        auto [e, f, g, h] = other.data;

        data[0] = a * e + b * g;
        data[1] = a * f + b * h;
        data[2] = c * e + d * g;
        data[3] = c * f + d * h;
        /*
        不能这样写，如果是自身赋值，，temp *= temp会导致计算结果错误，因为temp被修改了，也就是other.data被修改了
        auto [a, b, c, d] = data;
        data[0] = a * other.data[0] + b * other.data[2];
        data[1] = a * other.data[1] + b * other.data[3];
        data[2] = c * other.data[0] + d * other.data[2];
        data[3] = c * other.data[1] + d * other.data[3];
        */
        return *this;
    }

    static constexpr Matrix2D matrixPower(const Matrix2D& base, Ty exponent) {
        Matrix2D<Ty> result{ 1, 0, 0, 1 };
        Matrix2D<Ty> temp = base;
        Ty n = exponent;
        while (n > 0) {
            if (n & 1) result *= temp;
            temp *= temp;
            n >>= 1;
        }
        return result;
    }
};
constexpr std::size_t fibonacci(std::size_t n) {
    if (n == 0) return 0;
    return Matrix2D<std::size_t>::matrixPower(Matrix2D<std::size_t>{1, 1, 1, 0}, n - 1).data[0];
}
int main() {
    std::cout.setf(std::ios_base::boolalpha);
    constexpr auto x = fibonacci(4);
    //filter_sin_greater_than_zero();
    //benchmark_sort();
    //avx_test();

    //// 测试不同大小的矩阵
    //std::vector<std::vector<int>> testMatrix = {
    //    {1, 2, 3},
    //    {4, 5, 6},
    //    {7, 8, 9}
    //};

    //std::cout << "原始矩阵 (3x3 奇数):\n";
    //for (const auto& row : testMatrix) {
    //    for (int val : row) {
    //        std::cout << val << " ";
    //    }
    //    std::cout << "\n";
    //}

    //auto rotated = testMatrix;
    //rotateMatrixDirect(rotated);

    //std::cout << "\n旋转后矩阵:\n";
    //for (const auto& row : rotated) {
    //    for (int val : row) {
    //        std::cout << val << " ";
    //    }
    //    std::cout << "\n";
    //}

    //// 运行性能测试
    //MatrixRotateBenchmark benchmark;
    //benchmark.runBenchmark();

    //QuickSortBenchmark::run_benchmark<int>(100000);
    //radix_sort_test();
    return 0;
}