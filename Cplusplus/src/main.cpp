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


int main() {
    using namespace std::literals;
    std::cout.setf(std::ios_base::boolalpha);
    return 0;
}
