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
#include "sort/benchmark_parallel_sort.hpp"
#include "sort/radix_sort.hpp"
#include "parallel/avx_filter.hpp"
#include "leetcode/0048_rotate_image.h"
#include "leetcode/0912_sort_array.h"
using std::string;
using std::vector;
using std::queue;
using namespace std::literals;

int main() {
    std::cout.setf(std::ios_base::boolalpha);
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
    radix_sort_test();
    return 0;
}
