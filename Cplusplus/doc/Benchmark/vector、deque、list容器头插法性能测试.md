# CPU硬件缓存参数

```
Run on (8 X 2400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 8192 KiB (x1)
```



# 性能测试代码

```C++
#include <benchmark/benchmark.h>
#include <vector>
#include <list>
#include <deque>
#include <algorithm>
#include <iostream>

// 测试 vector 头插性能
static void BM_VectorFrontInsert(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();  // 暂停计时
        std::vector<int> vec;
        // 预分配内存以避免重新分配的影响（如果要测试重新分配的影响，可以注释掉这行）
        vec.reserve(state.range(0) + 1);
        state.ResumeTiming();  // 恢复计时

        for (int i = 0; i < state.range(0); ++i) {
            vec.insert(vec.begin(), i);
        }

        benchmark::DoNotOptimize(vec);
    }
    state.SetComplexityN(state.range(0));
}

// 测试 vector 头插（使用反向迭代器技巧）
static void BM_VectorFrontInsertReverse(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<int> vec;
        vec.reserve(state.range(0) + 1);
        state.ResumeTiming();

        for (int i = 0; i < state.range(0); ++i) {
            vec.push_back(i);
        }
        std::reverse(vec.begin(), vec.end());

        benchmark::DoNotOptimize(vec);
    }
    state.SetComplexityN(state.range(0));
}

// 测试 list 头插性能
static void BM_ListFrontInsert(benchmark::State& state) {
    for (auto _ : state) {
        std::list<int> lst;

        for (int i = 0; i < state.range(0); ++i) {
            lst.push_front(i);
        }

        benchmark::DoNotOptimize(lst);
    }
    state.SetComplexityN(state.range(0));
}

// 测试 deque 头插性能作为对比
static void BM_DequeFrontInsert(benchmark::State& state) {
    for (auto _ : state) {
        std::deque<int> dq;

        for (int i = 0; i < state.range(0); ++i) {
            dq.push_front(i);
        }

        benchmark::DoNotOptimize(dq);
    }
    state.SetComplexityN(state.range(0));
}

// 综合测试：从不同规模测试性能对比
static void BM_CompareFrontInsert(benchmark::State& state) {
    int operation = state.range(0);  // 操作类型
    int size = state.range(1);       // 容器大小

    for (auto _ : state) {
        state.PauseTiming();

        if (operation == 0) {  // vector
            std::vector<int> vec;
            vec.reserve(size + 1);
            state.ResumeTiming();

            for (int i = 0; i < size; ++i) {
                vec.insert(vec.begin(), i);
            }
            benchmark::DoNotOptimize(vec);

        }
        else if (operation == 1) {  // list
            std::list<int> lst;
            state.ResumeTiming();

            for (int i = 0; i < size; ++i) {
                lst.push_front(i);
            }
            benchmark::DoNotOptimize(lst);

        }
        else if (operation == 2) {  // deque
            std::deque<int> dq;
            state.ResumeTiming();

            for (int i = 0; i < size; ++i) {
                dq.push_front(i);
            }
            benchmark::DoNotOptimize(dq);
        }
    }
}

// 注册基准测试
// 测试从小到大的不同规模
BENCHMARK(BM_VectorFrontInsert)
->RangeMultiplier(2)
->Range(8, 1 << 20)  // 从8到1M
->Complexity(benchmark::oN);  // 设置复杂度为O(n)

BENCHMARK(BM_ListFrontInsert)
->RangeMultiplier(2)
->Range(8, 1 << 20)
->Complexity(benchmark::o1);  // list头插是O(1)

BENCHMARK(BM_DequeFrontInsert)
->RangeMultiplier(2)
->Range(8, 1 << 20)
->Complexity(benchmark::o1);  // deque头插也是O(1)

BENCHMARK(BM_VectorFrontInsertReverse)
->RangeMultiplier(2)
->Range(8, 1 << 20)
->Complexity(benchmark::oN);

// 固定几个关键规模进行详细对比
static void CustomArguments(benchmark::internal::Benchmark* b) {
    // 测试几个关键转折点
    const std::vector<int> sizes = { 10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000 };

    for (int op = 0; op < 3; ++op) {  // 0: vector, 1: list, 2: deque
        for (int size : sizes) {
            b->Args({ op, size });
        }
    }
}

BENCHMARK(BM_CompareFrontInsert)->Apply(CustomArguments);

// 自定义主函数来输出分析
int main(int argc, char** argv) {
    // 输出一些理论分析
    std::cout << "性能分析理论:\n";
    std::cout << "1. vector头插: O(n)复杂度，需要移动所有元素\n";
    std::cout << "   但内存连续，缓存友好，小数据量时可能更快\n";
    std::cout << "2. list头插: O(1)复杂度，只修改指针\n";
    std::cout << "   但内存不连续，缓存不友好\n";
    std::cout << "3. deque头插: O(1)复杂度，分段连续\n\n";
    std::cout << "预期: 在小数据量(<1000)时vector可能更快\n";
    std::cout << "      大数据量时list会反超\n\n";

    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();

    return 0;
}
```

# 性能测试结果

```
Run on (8 X 2400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 8192 KiB (x1)
------------------------------------------------------------------------------
Benchmark                                    Time             CPU   Iterations
------------------------------------------------------------------------------
BM_VectorFrontInsert/8                     196 ns          184 ns      3733333
BM_VectorFrontInsert/16                    240 ns          230 ns      2240000
BM_VectorFrontInsert/32                    328 ns          342 ns      2240000
BM_VectorFrontInsert/64                    568 ns          578 ns      1000000
BM_VectorFrontInsert/128                  1320 ns         1350 ns       497778
BM_VectorFrontInsert/256                  3488 ns         3610 ns       194783
BM_VectorFrontInsert/512                 11249 ns        11230 ns        64000
BM_VectorFrontInsert/1024                39082 ns        38504 ns        18667
BM_VectorFrontInsert/2048               144258 ns       144385 ns         4978
BM_VectorFrontInsert/4096               552207 ns       562500 ns         1000
BM_VectorFrontInsert/8192              2184250 ns      2197266 ns          320
BM_VectorFrontInsert/16384            10914091 ns     10986328 ns           64
BM_VectorFrontInsert/32768            45959993 ns     45833333 ns           15
BM_VectorFrontInsert/65536           187379200 ns    187500000 ns            4
BM_VectorFrontInsert/131072          868977200 ns    859375000 ns            1
BM_VectorFrontInsert/262144         3714362800 ns   3718750000 ns            1
BM_VectorFrontInsert/524288         1.5122e+10 ns   1.5078e+10 ns            1
BM_VectorFrontInsert/1048576        6.3920e+10 ns   6.3688e+10 ns            1
BM_VectorFrontInsert_BigO             51878.43 N      51696.55 N
BM_VectorFrontInsert_RMS                    99 %            99 %
BM_ListFrontInsert/8                       450 ns          445 ns      1544828
BM_ListFrontInsert/16                      839 ns          837 ns       896000
BM_ListFrontInsert/32                     2184 ns         2148 ns       320000
BM_ListFrontInsert/64                     3814 ns         3767 ns       186667
BM_ListFrontInsert/128                    8341 ns         8370 ns        74667
BM_ListFrontInsert/256                   15540 ns        15346 ns        44800
BM_ListFrontInsert/512                   30646 ns        30762 ns        24889
BM_ListFrontInsert/1024                  58470 ns        58594 ns        11200
BM_ListFrontInsert/2048                 111084 ns       112305 ns         6400
BM_ListFrontInsert/4096                 226426 ns       228795 ns         2800
BM_ListFrontInsert/8192                 438776 ns       439453 ns         1600
BM_ListFrontInsert/16384                867350 ns       871931 ns          896
BM_ListFrontInsert/32768               1757340 ns      1759383 ns          373
BM_ListFrontInsert/65536               3561677 ns      3525641 ns          195
BM_ListFrontInsert/131072              7373049 ns      7465278 ns           90
BM_ListFrontInsert/262144             15591804 ns     15625000 ns           45
BM_ListFrontInsert/524288             31670748 ns     31994048 ns           21
BM_ListFrontInsert/1048576            65903845 ns     65340909 ns           11
BM_ListFrontInsert_BigO             7090132.41 (1)  7082389.51 (1)
BM_ListFrontInsert_RMS                     229 %           228 %
BM_DequeFrontInsert/8                      237 ns          235 ns      2986667
BM_DequeFrontInsert/16                     382 ns          384 ns      1792000
BM_DequeFrontInsert/32                     680 ns          684 ns      1120000
BM_DequeFrontInsert/64                    1232 ns         1228 ns       560000
BM_DequeFrontInsert/128                   2879 ns         2905 ns       263529
BM_DequeFrontInsert/256                   4934 ns         5000 ns       100000
BM_DequeFrontInsert/512                   9828 ns         9835 ns        74667
BM_DequeFrontInsert/1024                 18965 ns        18834 ns        37333
BM_DequeFrontInsert/2048                 37895 ns        37667 ns        18667
BM_DequeFrontInsert/4096                 72483 ns        73242 ns         8960
BM_DequeFrontInsert/8192                138393 ns       138108 ns         4978
BM_DequeFrontInsert/16384               265922 ns       260911 ns         2635
BM_DequeFrontInsert/32768               522517 ns       516183 ns         1120
BM_DequeFrontInsert/65536              1043400 ns      1045850 ns          747
BM_DequeFrontInsert/131072             2079811 ns      2083333 ns          345
BM_DequeFrontInsert/262144             4484808 ns      4464286 ns          154
BM_DequeFrontInsert/524288             9391350 ns      9277344 ns           64
BM_DequeFrontInsert/1048576           19175365 ns     18841912 ns           34
BM_DequeFrontInsert_BigO            2069504.47 (1)  2043218.88 (1)
BM_DequeFrontInsert_RMS                    229 %           228 %
BM_VectorFrontInsertReverse/8              178 ns          172 ns      3733333
BM_VectorFrontInsertReverse/16             184 ns          220 ns      4480000
BM_VectorFrontInsertReverse/32             215 ns          190 ns      3200000
BM_VectorFrontInsertReverse/64             258 ns          251 ns      2986667
BM_VectorFrontInsertReverse/128            358 ns          361 ns      1947826
BM_VectorFrontInsertReverse/256            540 ns          562 ns      1000000
BM_VectorFrontInsertReverse/512            923 ns          942 ns       896000
BM_VectorFrontInsertReverse/1024          1721 ns         1779 ns       448000
BM_VectorFrontInsertReverse/2048          3217 ns         3223 ns       213333
BM_VectorFrontInsertReverse/4096          6268 ns         6452 ns        89600
BM_VectorFrontInsertReverse/8192         12289 ns        12556 ns        56000
BM_VectorFrontInsertReverse/16384        24496 ns        23996 ns        28000
BM_VectorFrontInsertReverse/32768        49814 ns        50000 ns        10000
BM_VectorFrontInsertReverse/65536        98096 ns        98349 ns         7467
BM_VectorFrontInsertReverse/131072      200055 ns       200911 ns         3733
BM_VectorFrontInsertReverse/262144      565155 ns       530134 ns         1120
BM_VectorFrontInsertReverse/524288     1151109 ns      1171875 ns          640
BM_VectorFrontInsertReverse/1048576    2335212 ns      2343750 ns          320
BM_VectorFrontInsertReverse_BigO          2.21 N          2.21 N
BM_VectorFrontInsertReverse_RMS             10 %            11 %
BM_CompareFrontInsert/0/10                 199 ns          194 ns      2986667
BM_CompareFrontInsert/0/50                 441 ns          429 ns      1493333
BM_CompareFrontInsert/0/100                929 ns          879 ns       746667
BM_CompareFrontInsert/0/500              10713 ns        10882 ns        74667
BM_CompareFrontInsert/0/1000             37338 ns        37703 ns        19478
BM_CompareFrontInsert/0/5000            816674 ns       802176 ns          896
BM_CompareFrontInsert/0/10000          3506640 ns      3525641 ns          195
BM_CompareFrontInsert/0/50000        107687400 ns    106770833 ns            6
BM_CompareFrontInsert/0/100000       474532700 ns    468750000 ns            2
BM_CompareFrontInsert/1/10                 661 ns          703 ns      1000000
BM_CompareFrontInsert/1/50                3151 ns         3013 ns       248889
BM_CompareFrontInsert/1/100               6141 ns         5580 ns       112000
BM_CompareFrontInsert/1/500              30073 ns        30483 ns        23579
BM_CompareFrontInsert/1/1000             56868 ns        56250 ns        10000
BM_CompareFrontInsert/1/5000            262281 ns       254981 ns         2635
BM_CompareFrontInsert/1/10000           515132 ns       516183 ns         1120
BM_CompareFrontInsert/1/50000          2586098 ns      2604167 ns          264
BM_CompareFrontInsert/1/100000         5297188 ns      5301339 ns          112
BM_CompareFrontInsert/2/10                 413 ns          443 ns      1659259
BM_CompareFrontInsert/2/50                1101 ns         1116 ns       560000
BM_CompareFrontInsert/2/100               2396 ns         2550 ns       263529
BM_CompareFrontInsert/2/500               9910 ns        10010 ns        64000
BM_CompareFrontInsert/2/1000             18890 ns        19252 ns        37333
BM_CompareFrontInsert/2/5000             85838 ns        85794 ns         7467
BM_CompareFrontInsert/2/10000           166702 ns       168795 ns         4073
BM_CompareFrontInsert/2/50000           780099 ns       784738 ns          896
BM_CompareFrontInsert/2/100000         1574778 ns      1534598 ns          448
```

# 头插法性能分析报告

## 实验概述
本测试对比了三种容器（vector、list、deque）在不同规模下的头插法性能。测试在具有以下缓存配置的系统上进行：
- L1 Data Cache: 32 KiB (x4)
- L2 Cache: 256 KiB (x4) 
- L3 Cache: 8192 KiB (x1)

## 关键发现

### 1. 性能转折点
基于测试数据，可以观察到以下转折点：

| 元素数量  | vector头插   | list头插     | 性能对比            |
| --------- | ------------ | ------------ | ------------------- |
| 8-64个    | 184-578ns    | 445-3767ns   | **vector明显更快**  |
| 128-512个 | 1350-11230ns | 8370-30762ns | **vector仍更快**    |
| 1024个    | 38504ns      | 58594ns      | vector略快(66%耗时) |
| 2048个    | 144385ns     | 112305ns     | **list开始反超**    |
| 4096个    | 562500ns     | 228795ns     | list快约2.5倍       |

**结论：在约1000-2000个元素处出现性能转折点。**

### 2. 缓存友好性的影响
测试结果验证了缓存局部性对性能的巨大影响：
- **小数据量**（<1000个int）：vector完全在L1/L2缓存中，即使需要移动元素，缓存命中率高，性能优于list
- **中等数据量**（1000-4000个int）：数据开始超出L1/L2缓存，vector的缓存优势减弱
- **大数据量**（>4000个int）：vector数据频繁在内存和缓存间交换，list的O(1)优势明显

### 3. 各容器具体表现

#### vector头插（标准方法）
- **复杂度**：O(n)，与理论一致
- **特点**：小数据量极快，大数据量急剧恶化
- **缓存优势范围**：约0-1500个int元素

#### vector头插（反向插入法）
- **性能**：显著优于标准头插法，在所有规模下都保持良好性能
- **原因**：先push_back（O(1)摊销），再reverse（O(n)但缓存友好）
- **推荐**：如果需要头插效果，这是最佳实践

#### list头插
- **复杂度**：O(1)，但常数因子大
- **特点**：小数据量因缓存不友好和内存分配开销较慢，大数据量稳定
- **转折点**：约1500个元素后开始明显优于vector标准头插

#### deque头插
- **性能**：介于vector和list之间
- **特点**：O(1)复杂度，分段连续存储平衡了缓存友好性和插入效率
- **适用场景**：需要频繁两端操作的场景

## 缓存分析与性能解释

### L1缓存分析
- **容量**：32KB ≈ 8192个int
- **观察**：当vector元素数量<8000时，大部分数据能留在L1缓存
- **实际表现**：vector在<1500个元素时仍保持优势，说明缓存预取有效

### L2缓存分析
- **容量**：256KB ≈ 65536个int
- **观察**：vector在65536个元素时耗时已显著增加，但仍比预期慢
- **解释**：虽然数据可能仍在L2缓存，但移动操作的O(n²)复杂度主导性能

### L3缓存影响
- **容量**：8MB ≈ 2百万个int
- **观察**：list在大数据量时优势明显，但vector反向插入法仍表现良好
- **结论**：算法复杂度比缓存影响更重要

## 推荐策略

### 1. 根据数据规模选择容器

| 元素数量   | 推荐方法         | 理由           |
| ---------- | ---------------- | -------------- |
| < 500个    | vector标准头插   | 缓存优势明显   |
| 500-2000个 | vector反向插入法 | 最佳平衡       |
| > 2000个   | list或deque      | O(1)复杂度优势 |

### 2. 实际应用建议
1. **优先考虑vector反向插入法**：除非有特殊原因，这是最通用高效的方案
2. **预分配内存**：vector使用reserve()可避免重新分配开销
3. **考虑访问模式**：如果后续需要随机访问，vector更优
4. **元素大小影响**：本测试基于int(4字节)，对于更大对象，vector优势范围会缩小

### 3. 性能优化启示
1. **缓存友好性在小数据量时压倒复杂度优势**
2. **O(1)复杂度在大数据量时才真正体现价值**
3. **算法改进（如反向插入）可能比容器选择更重要**

## 结论

在具有32KB L1/256KB L2/8MB L3缓存的系统上：
1. **vector在小数据量（<1500个int）时因缓存友好性表现最佳**
2. **list在大数据量（>2000个int）时因O(1)复杂度获胜**
3. **deque提供平衡的性能，适合两端操作场景**
4. **vector反向插入法是最优实践，在所有规模下都有良好表现**

**核心教训**：不应仅凭理论复杂度选择容器，必须考虑实际数据规模、缓存体系和使用模式。在小数据量场景中，缓存局部性的价值可能超过算法复杂度的优势。