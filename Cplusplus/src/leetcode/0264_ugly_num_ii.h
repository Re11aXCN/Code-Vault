#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>

/*
 * @lc app=leetcode.cn id=264 lang=cpp
 *
 * [264] 丑数 II
 */

constexpr int CACHE_SIZE = 1690;

// 版本1：动态数组 + 循环展开
int nthUglyNumber_v1(int n) {
    if (n <= 0) return 0;

    std::vector<int> ugly(n);
    ugly[0] = 1;
    int i2 = 0, i3 = 0, i5 = 0;

    // 循环展开：每次处理4个元素
    int i = 1;
    for (; i + 3 < n; i += 4) {
        // 预先加载可能用到的值到寄存器
        int val2 = ugly[i2], val3 = ugly[i3], val5 = ugly[i5];

        // 处理第1个元素
        int next2_1 = val2 * 2;
        int next3_1 = val3 * 3;
        int next5_1 = val5 * 5;
        ugly[i] = std::min(next2_1, std::min(next3_1, next5_1));

        // 更新指针（可能需要多次更新）
        if (ugly[i] == next2_1) { i2++; val2 = ugly[i2]; }
        if (ugly[i] == next3_1) { i3++; val3 = ugly[i3]; }
        if (ugly[i] == next5_1) { i5++; val5 = ugly[i5]; }

        // 处理第2个元素
        int next2_2 = val2 * 2;
        int next3_2 = val3 * 3;
        int next5_2 = val5 * 5;
        ugly[i + 1] = std::min(next2_2, std::min(next3_2, next5_2));

        if (ugly[i + 1] == next2_2) { i2++; val2 = ugly[i2]; }
        if (ugly[i + 1] == next3_2) { i3++; val3 = ugly[i3]; }
        if (ugly[i + 1] == next5_2) { i5++; val5 = ugly[i5]; }

        // 处理第3个元素
        int next2_3 = val2 * 2;
        int next3_3 = val3 * 3;
        int next5_3 = val5 * 5;
        ugly[i + 2] = std::min(next2_3, std::min(next3_3, next5_3));

        if (ugly[i + 2] == next2_3) { i2++; val2 = ugly[i2]; }
        if (ugly[i + 2] == next3_3) { i3++; val3 = ugly[i3]; }
        if (ugly[i + 2] == next5_3) { i5++; val5 = ugly[i5]; }

        // 处理第4个元素
        int next2_4 = val2 * 2;
        int next3_4 = val3 * 3;
        int next5_4 = val5 * 5;
        ugly[i + 3] = std::min(next2_4, std::min(next3_4, next5_4));

        if (ugly[i + 3] == next2_4) i2++;
        if (ugly[i + 3] == next3_4) i3++;
        if (ugly[i + 3] == next5_4) i5++;
    }

    // 处理剩余的元素
    for (; i < n; i++) {
        ugly[i] = std::min(ugly[i2] * 2,
            std::min(ugly[i3] * 3, ugly[i5] * 5));

        if (ugly[i] == ugly[i2] * 2) i2++;
        if (ugly[i] == ugly[i3] * 3) i3++;
        if (ugly[i] == ugly[i5] * 5) i5++;
    }

    return ugly[n - 1];
}

// 版本2：动态数组 + 标准循环
int nthUglyNumber_v2(int n) {
    if (n <= 0) return 0;

    std::vector<int> ugly(n);
    ugly[0] = 1;
    int i2 = 0, i3 = 0, i5 = 0;

    for (int i = 1; i < n; i++) {
        int candidate2 = ugly[i2] * 2;
        int candidate3 = ugly[i3] * 3;
        int candidate5 = ugly[i5] * 5;

        ugly[i] = candidate2 < candidate3 ?
            (candidate2 < candidate5 ? candidate2 : candidate5) :
            (candidate3 < candidate5 ? candidate3 : candidate5);

        if (ugly[i] == candidate2) i2++;
        if (ugly[i] == candidate3) i3++;
        if (ugly[i] == candidate5) i5++;
    }

    return ugly[n - 1];
}

// 版本3：静态数组 + 标准循环
int nthUglyNumber_v3(int n) {
    if (n <= 0 || n > CACHE_SIZE) return 0;

    static int ugly[CACHE_SIZE];
    ugly[0] = 1;

    int i2 = 0, i3 = 0, i5 = 0;
    // 逻辑太复杂编译器不敢循环展开
#pragma GCC unroll 4 
    for (int i = 1; i < n; i++) {
        int candidate2 = ugly[i2] * 2;
        int candidate3 = ugly[i3] * 3;
        int candidate5 = ugly[i5] * 5;

        ugly[i] = candidate2 < candidate3 ?
            (candidate2 < candidate5 ? candidate2 : candidate5) :
            (candidate3 < candidate5 ? candidate3 : candidate5);

        if (ugly[i] == candidate2) i2++;
        if (ugly[i] == candidate3) i3++;
        if (ugly[i] == candidate5) i5++;
    }

    return ugly[n - 1];
}

// 版本4：静态数组 + 8次循环展开 + 更激进的寄存器优化
int nthUglyNumber_v4(int n) {
    if (n <= 0 || n > CACHE_SIZE) return 0;

    static int ugly[CACHE_SIZE];
    ugly[0] = 1;
    int i2 = 0, i3 = 0, i5 = 0;

    // 8次循环展开
    int i = 1;
    for (; i + 7 < n; i += 8) {
        // 预先加载所有可能用到的值
        int val2 = ugly[i2], val3 = ugly[i3], val5 = ugly[i5];

        // 处理8个元素
        for (int j = 0; j < 8; j++) {
            int next2 = val2 * 2;
            int next3 = val3 * 3;
            int next5 = val5 * 5;

            ugly[i + j] = next2 < next3 ?
                (next2 < next5 ? next2 : next5) :
                (next3 < next5 ? next3 : next5);

            // 更新指针和值
            if (ugly[i + j] == next2) {
                i2++;
                val2 = ugly[i2];
            }
            if (ugly[i + j] == next3) {
                i3++;
                val3 = ugly[i3];
            }
            if (ugly[i + j] == next5) {
                i5++;
                val5 = ugly[i5];
            }
        }
    }

    // 处理剩余元素
    for (; i < n; i++) {
        int candidate2 = ugly[i2] * 2;
        int candidate3 = ugly[i3] * 3;
        int candidate5 = ugly[i5] * 5;

        ugly[i] = candidate2 < candidate3 ?
            (candidate2 < candidate5 ? candidate2 : candidate5) :
            (candidate3 < candidate5 ? candidate3 : candidate5);

        if (ugly[i] == candidate2) i2++;
        if (ugly[i] == candidate3) i3++;
        if (ugly[i] == candidate5) i5++;
    }

    return ugly[n - 1];
}

// 版本5：静态数组 + 4次循环展开 + 手动优化比较
int nthUglyNumber_v5(int n) {
    if (n <= 0 || n > CACHE_SIZE) return 0;

    static int ugly[CACHE_SIZE];
    ugly[0] = 1;
    int i2 = 0, i3 = 0, i5 = 0;

    int i = 1;
    for (; i + 3 < n; i += 4) {
        int val2 = ugly[i2], val3 = ugly[i3], val5 = ugly[i5];

        // 手动处理4个元素，避免函数调用开销
        for (int j = 0; j < 4; j++) {
            int next2 = val2 * 2;
            int next3 = val3 * 3;
            int next5 = val5 * 5;

            int min_val = next2;
            if (next3 < min_val) min_val = next3;
            if (next5 < min_val) min_val = next5;

            ugly[i + j] = min_val;

            if (min_val == next2) { i2++; val2 = ugly[i2]; }
            if (min_val == next3) { i3++; val3 = ugly[i3]; }
            if (min_val == next5) { i5++; val5 = ugly[i5]; }
        }
    }

    for (; i < n; i++) {
        int next2 = ugly[i2] * 2;
        int next3 = ugly[i3] * 3;
        int next5 = ugly[i5] * 5;

        int min_val = next2;
        if (next3 < min_val) min_val = next3;
        if (next5 < min_val) min_val = next5;

        ugly[i] = min_val;

        if (min_val == next2) i2++;
        if (min_val == next3) i3++;
        if (min_val == next5) i5++;
    }

    return ugly[n - 1];
}

// 版本6：静态数组 + 16次循环展开 + 预计算优化
int nthUglyNumber_v6(int n) {
    if (n <= 0 || n > CACHE_SIZE) return 0;

    static int ugly[CACHE_SIZE];
    ugly[0] = 1;
    int i2 = 0, i3 = 0, i5 = 0;

    int i = 1;
    for (; i + 15 < n; i += 16) {
        // 批量预加载值
        int val2 = ugly[i2], val3 = ugly[i3], val5 = ugly[i5];

        for (int j = 0; j < 16; j++) {
            int next2 = val2 * 2;
            int next3 = val3 * 3;
            int next5 = val5 * 5;

            // 手动内联min函数
            int min_val = (next2 < next3) ?
                ((next2 < next5) ? next2 : next5) :
                ((next3 < next5) ? next3 : next5);

            ugly[i + j] = min_val;

            // 条件更新
            if (min_val == next2) {
                i2++;
                val2 = ugly[i2];
            }
            if (min_val == next3) {
                i3++;
                val3 = ugly[i3];
            }
            if (min_val == next5) {
                i5++;
                val5 = ugly[i5];
            }
        }
    }

    // 处理剩余元素
    for (; i < n; i++) {
        int next2 = ugly[i2] * 2;
        int next3 = ugly[i3] * 3;
        int next5 = ugly[i5] * 5;

        int min_val = (next2 < next3) ?
            ((next2 < next5) ? next2 : next5) :
            ((next3 < next5) ? next3 : next5);

        ugly[i] = min_val;

        if (min_val == next2) i2++;
        if (min_val == next3) i3++;
        if (min_val == next5) i5++;
    }

    return ugly[n - 1];
}

// 验证函数正确性
bool verifyCorrectness() {
    std::cout << "Verifying correctness..." << std::endl;
    for (int i = 1; i <= 100; i++) {
        int r1 = nthUglyNumber_v1(i);
        int r2 = nthUglyNumber_v2(i);
        int r3 = nthUglyNumber_v3(i);
        int r4 = nthUglyNumber_v4(i);
        int r5 = nthUglyNumber_v5(i);
        int r6 = nthUglyNumber_v6(i);

        if (r1 != r2 || r1 != r3 || r1 != r4 || r1 != r5 || r1 != r6) {
            std::cout << "Error at n=" << i << ": "
                << r1 << "," << r2 << "," << r3 << ","
                << r4 << "," << r5 << "," << r6 << std::endl;
            return false;
        }
    }
    std::cout << "All tests passed! All versions produce same results." << std::endl;
    return true;
}

// 性能测试函数
void performanceTest() {
    const int n = 1690;
    const int iterations = 10000;

    std::cout << "\nPerformance Test: n=" << n << ", iterations=" << iterations << std::endl;
    std::cout << "==========================================" << std::endl;

    auto testVersion = [&](auto func, const std::string& name) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; i++) {
            volatile int result = func(n);
        }
        auto end = std::chrono::high_resolution_clock::now();
        return duration_cast<std::chrono::microseconds>(end - start).count();
        };

    long long time1 = testVersion(nthUglyNumber_v1, "v1");
    long long time2 = testVersion(nthUglyNumber_v2, "v2");
    long long time3 = testVersion(nthUglyNumber_v3, "v3");
    long long time4 = testVersion(nthUglyNumber_v4, "v4");
    long long time5 = testVersion(nthUglyNumber_v5, "v5");
    long long time6 = testVersion(nthUglyNumber_v6, "v6");

    std::cout << "v1 (Dynamic + 4-unroll): " << time1 << " μs" << std::endl;
    std::cout << "v2 (Dynamic + standard): " << time2 << " μs" << std::endl;
    std::cout << "v3 (Static  + standard): " << time3 << " μs" << std::endl;
    std::cout << "v4 (Static  + 8-unroll): " << time4 << " μs" << std::endl;
    std::cout << "v5 (Static  + 4-unroll): " << time5 << " μs" << std::endl;
    std::cout << "v6 (Static  + 16-unroll): " << time6 << " μs" << std::endl;

    // 找出最快版本
    long long min_time = std::min({ time1, time2, time3, time4, time5, time6 });
    std::vector<std::pair<std::string, long long>> versions = {
        {"v1", time1}, {"v2", time2}, {"v3", time3},
        {"v4", time4}, {"v5", time5}, {"v6", time6}
    };

    std::cout << "\nPerformance Ranking:" << std::endl;
    std::cout << "====================" << std::endl;
    sort(versions.begin(), versions.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    for (size_t i = 0; i < versions.size(); i++) {
        double speedup = (double)versions[i].second / min_time;
        std::cout << i + 1 << ". " << versions[i].first << ": "
            << versions[i].second << " μs ("
            << speedup << "x slower than fastest)" << std::endl;
    }
}

// 详细性能分析
void detailedPerformanceTest() {
    const int n = 1690;
    const int warmup = 1000;
    const int test_iterations = 50000;

    std::cout << "\nDetailed Performance Analysis (nanoseconds per call):" << std::endl;
    std::cout << "====================================================" << std::endl;

    // 预热
    for (int i = 0; i < warmup; i++) {
        nthUglyNumber_v1(n);
        nthUglyNumber_v2(n);
        nthUglyNumber_v3(n);
        nthUglyNumber_v4(n);
        nthUglyNumber_v5(n);
        nthUglyNumber_v6(n);
    }

    auto testVersionDetailed = [&](auto func, const std::string& name) {
        long long total = 0;
        for (int i = 0; i < test_iterations; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            volatile int result = func(n);
            auto end = std::chrono::high_resolution_clock::now();
            total += duration_cast<std::chrono::nanoseconds>(end - start).count();
        }
        return total / (double)test_iterations;
        };

    double avg1 = testVersionDetailed(nthUglyNumber_v1, "v1");
    double avg2 = testVersionDetailed(nthUglyNumber_v2, "v2");
    double avg3 = testVersionDetailed(nthUglyNumber_v3, "v3");
    double avg4 = testVersionDetailed(nthUglyNumber_v4, "v4");
    double avg5 = testVersionDetailed(nthUglyNumber_v5, "v5");
    double avg6 = testVersionDetailed(nthUglyNumber_v6, "v6");

    std::cout << std::fixed;
    std::cout.precision(2);
    std::cout << "v1 (Dynamic + 4-unroll): " << avg1 << " ns" << std::endl;
    std::cout << "v2 (Dynamic + standard): " << avg2 << " ns" << std::endl;
    std::cout << "v3 (Static  + standard): " << avg3 << " ns" << std::endl;
    std::cout << "v4 (Static  + 8-unroll): " << avg4 << " ns" << std::endl;
    std::cout << "v5 (Static  + 4-unroll): " << avg5 << " ns" << std::endl;
    std::cout << "v6 (Static  + 16-unroll): " << avg6 << " ns" << std::endl;

    double min_avg = std::min({ avg1, avg2, avg3, avg4, avg5, avg6 });
    std::cout << "\nFastest version is " << (min_avg == avg1 ? "v1" :
        min_avg == avg2 ? "v2" :
        min_avg == avg3 ? "v3" :
        min_avg == avg4 ? "v4" :
        min_avg == avg5 ? "v5" : "v6")
        << " with " << min_avg << " ns per call" << std::endl;
}



int ugly_num_unroll_performance_test() {
    std::cout << "Ugly Number Algorithm Performance Comparison" << std::endl;

    std::cout << "============================================" << std::endl;

    // 首先验证正确性
    if (!verifyCorrectness()) {
        return 1;
    }

    // 运行性能测试
    performanceTest();

    // 运行详细性能分析
    detailedPerformanceTest();

    return 0;
}

/*
Ugly Number Algorithm Performance Comparison
============================================
Verifying correctness...
All tests passed! All versions produce same results.

Performance Test: n=1690, iterations=10000
==========================================
v1 (Dynamic + 4-unroll): 34759 μs
v2 (Dynamic + standard): 55235 μs
v3 (Static  + standard): 55305 μs
v4 (Static  + 8-unroll): 30152 μs
v5 (Static  + 4-unroll): 30559 μs
v6 (Static  + 16-unroll): 27971 μs

Performance Ranking:
====================
1. v6: 27971 μs (1x slower than fastest)
2. v4: 30152 μs (1.07797x slower than fastest)
3. v5: 30559 μs (1.09252x slower than fastest)
4. v1: 34759 μs (1.24268x slower than fastest)
5. v2: 55235 μs (1.97472x slower than fastest)
6. v3: 55305 μs (1.97723x slower than fastest)

Detailed Performance Analysis (nanoseconds per call):
====================================================
v1 (Dynamic + 4-unroll): 3415.92 ns
v2 (Dynamic + standard): 5688.32 ns
v3 (Static  + standard): 5761.61 ns
v4 (Static  + 8-unroll): 5119.66 ns
v5 (Static  + 4-unroll): 3523.98 ns
v6 (Static  + 16-unroll): 3023.17 ns

Fastest version is v6 with 3023.17 ns per call

*/