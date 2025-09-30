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
using std::string;
using std::vector;
using std::queue;

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8); // 输出 UTF-8 编码
#endif
    std::cout.setf(std::ios_base::boolalpha);
    auto benchmark = [](std::string_view name, auto&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        std::println("{} took {}us", name, std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
        };
    auto generate_test_data = [](size_t n, int min_val, int max_val) {
        if(n <= 1) throw std::invalid_argument("n must be greater than 1");
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(min_val, max_val);

        std::vector<int> data(n);
        for (size_t i = 0; i < n; ++i) {
            data[i] = dist(gen);
        }
        return data;
        };
    return 0;
}
