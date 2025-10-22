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
int main() {
    std::cout.setf(std::ios_base::boolalpha);
    filter_sin_greater_than_zero();
    benchmark_sort();
    return 0;
}
