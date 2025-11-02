#pragma once

// leetcode 912. Sort an Array
// 几种快速排序的实现版本，包括PDQSort和TimSort

#include <algorithm>
#include <functional>
#include <stack>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <cassert>
template<typename T>
class TimSort {
private:
    static const int MIN_MERGE = 32;

    static int minRunLength(int n) {
        int r = 0;
        while (n >= MIN_MERGE) {
            r |= (n & 1);
            n >>= 1;
        }
        return n + r;
    }

    static void insertionSort(T* data, int left, int right) {
        for (int i = left + 1; i <= right; ++i) {
            T temp = data[i];
            int j = i;
            while (j > left && data[j - 1] > temp) {
                data[j] = data[j - 1];
                --j;
            }
            data[j] = temp;
        }
    }

    static void merge(T* data, int left, int mid, int right, T* temp) {
        int i = left, j = mid + 1, k = left;

        // 复制到临时数组
        for (int idx = left; idx <= right; ++idx) {
            temp[idx] = data[idx];
        }

        // 合并
        while (i <= mid && j <= right) {
            if (temp[i] <= temp[j]) {
                data[k] = temp[i];
                ++i;
            }
            else {
                data[k] = temp[j];
                ++j;
            }
            ++k;
        }

        // 复制剩余元素
        while (i <= mid) {
            data[k] = temp[i];
            ++i;
            ++k;
        }
    }

public:
    static void sort(T* data, int n) {
        if (n <= 1) return;

        // 临时数组
        T* temp = new T[n];

        int minRun = minRunLength(n);

        // 排序小的run
        for (int i = 0; i < n; i += minRun) {
            int end = (i + minRun - 1 < n - 1) ? i + minRun - 1 : n - 1;
            insertionSort(data, i, end);
        }

        // 合并run
        for (int size = minRun; size < n; size = 2 * size) {
            for (int left = 0; left < n; left += 2 * size) {
                int mid = left + size - 1;
                if (mid >= n - 1) break;

                int right = (left + 2 * size - 1 < n - 1) ?
                    left + 2 * size - 1 : n - 1;

                if (data[mid] > data[mid + 1]) {
                    merge(data, left, mid, right, temp);
                }
            }
        }

        delete[] temp;
    }
};
template<typename T>
class PDQSort {
private:
    static const int INSERTION_SORT_THRESHOLD = 24;
    static const int NINTHER_THRESHOLD = 128;
    static const int PARTIAL_INSERTION_SORT_LIMIT = 8;

    static void insertionSort(T* data, int left, int right) {
        if (left >= right) return;

        for (int i = left + 1; i <= right; ++i) {
            T key = data[i];
            int j = i;

            while (j > left && data[j - 1] > key) {
                data[j] = data[j - 1];
                --j;
            }
            data[j] = key;
        }
    }

    static bool partialInsertionSort(T* data, int left, int right) {
        if (left >= right) return true;

        int limit = 0;
        for (int i = left + 1; i <= right; ++i) {
            if (limit > PARTIAL_INSERTION_SORT_LIMIT) return false;

            T key = data[i];
            int j = i;

            while (j > left && data[j - 1] > key) {
                data[j] = data[j - 1];
                --j;
                ++limit;
            }
            data[j] = key;
        }
        return true;
    }

    static void sort2(T& a, T& b) {
        if (b < a) std::swap(a, b);
    }

    static void sort3(T& a, T& b, T& c) {
        sort2(a, b);
        sort2(b, c);
        sort2(a, b);
    }

    static T* medianOf3(T* a, T* b, T* c) {
        if (*a < *b) {
            if (*b < *c) return b;
            else if (*a < *c) return c;
            else return a;
        }
        else {
            if (*a < *c) return a;
            else if (*b < *c) return c;
            else return b;
        }
    }

    static T* medianOf9(T* data, int left, int right) {
        int size = right - left + 1;
        int third = size / 3;

        T* a = data + left;
        T* b = data + left + third;
        T* c = data + left + 2 * third;

        T* med1 = medianOf3(a, a + third / 2, b);
        T* med2 = medianOf3(b, b + third / 2, c);
        T* med3 = medianOf3(c, c + third / 2, data + right);

        return medianOf3(med1, med2, med3);
    }

    static std::pair<int, bool> partitionRight(T* data, int left, int right) {
        T pivot = data[right];
        int i = left - 1;

        for (int j = left; j < right; ++j) {
            if (data[j] <= pivot) {
                ++i;
                std::swap(data[i], data[j]);
            }
        }
        std::swap(data[i + 1], data[right]);

        // 检查是否已经基本有序
        bool alreadyPartitioned = true;
        for (int j = left; j < i + 1; ++j) {
            if (data[j] > pivot) {
                alreadyPartitioned = false;
                break;
            }
        }

        return { i + 1, alreadyPartitioned };
    }

    static void heapSort(T* data, int left, int right) {
        std::make_heap(data + left, data + right + 1);
        std::sort_heap(data + left, data + right + 1);
    }

public:
    static void sort(T* data, int left, int right, int badAllowed = 0) {
        if (left >= right) return;

        const int size = right - left + 1;

        // 小数组使用插入排序
        if (size < INSERTION_SORT_THRESHOLD) {
            insertionSort(data, left, right);
            return;
        }

        // 限制递归深度，防止退化
        if (badAllowed == 0) {
            heapSort(data, left, right);
            return;
        }

        // 选择枢轴
        T* pivot;
        if (size > NINTHER_THRESHOLD) {
            pivot = medianOf9(data, left, right);
        }
        else {
            int mid = left + (right - left) / 2;
            pivot = medianOf3(data + left, data + mid, data + right);
        }

        // 将枢轴移到末尾
        std::swap(*pivot, data[right]);

        auto [pivotPos, alreadyPartitioned] = partitionRight(data, left, right);

        if (alreadyPartitioned) {
            return;
        }

        // 递归排序
        if (pivotPos - left > 16 && right - pivotPos > 16) {
            --badAllowed;
        }

        sort(data, left, pivotPos - 1, badAllowed);
        sort(data, pivotPos + 1, right, badAllowed);
    }

    static void sort(T* data, int n) {
        sort(data, 0, n - 1, int(log2(n)) * 2);
    }
};
template <typename T>
void hash_pivot_quick_sort(T* data, int left, int right) {
    if (left >= right) return;

    const int size = right - left + 1;
    if (size <= 32) {
        std::sort(data + left, data + right + 1);
        return;
    }

    // 使用哈希选择枢轴
    std::size_t mid = std::hash<int>{}(size);
    mid ^= std::hash<void*>{}(static_cast<void*>(data + left));
    mid %= size;
    mid += left;
    std::swap(data[left], data[mid]);

    T pivot = data[left];
    int i = left + 1, j = right;

    while (i <= j) {
        while (i <= j && data[i] < pivot) ++i;
        while (i <= j && data[j] > pivot) --j;
        if (i <= j) {
            std::swap(data[i], data[j]);
            ++i;
            --j;
        }
    }

    std::swap(data[left], data[j]);

    hash_pivot_quick_sort(data, left, j - 1);
    hash_pivot_quick_sort(data, j + 1, right);
}

template<typename T>
void lomuto_quick_sort(T* data, int left, int right) {
    if (left >= right) return;

    const int size = right - left + 1;
    if (size <= 32) {
        std::sort(data + left, data + right + 1);
        return;
    }

    // 选择中间元素作为枢轴
    int mid = left + (right - left) / 2;
    T pivot = data[mid];
    std::swap(data[mid], data[right]); // 移动枢轴到末尾

    int i = left - 1;

    // 分区操作
    for (int j = left; j < right; ++j) {
        if (data[j] <= pivot) {
            ++i;
            std::swap(data[i], data[j]);
        }
    }

    std::swap(data[i + 1], data[right]);

    // 递归排序
    lomuto_quick_sort(data, left, i);
    lomuto_quick_sort(data, i + 2, right);
}

template<typename T, typename Compare>
void hoare_quick_sort(T* data, int left, int right, Compare comp) {
    if (left >= right) return;

    const int size = right - left + 1;
    if (size <= 32) {
        std::sort(data + left, data + right + 1);
        return;
    }

    // 三数取中法选择枢轴, 确保 left ≤ mid ≤ right, 防止有序情况分布不均匀
    int mid = left + (right - left) / 2;
    if (comp(data[mid] , data[left])) std::swap(data[left], data[mid]);
    if (comp(data[right] , data[left])) std::swap(data[left], data[right]);
    if (comp(data[right] , data[mid])) std::swap(data[mid], data[right]);

    T pivot = data[mid];
    std::swap(data[mid], data[right - 1]); // 将枢轴放到倒数第二个位置

    int i = left, j = right - 1;
    while (true) {
        do { ++i; } while (i < right && comp(data[i], pivot));
        do { --j; } while (j > left && comp(pivot, data[j]));
        // 以下无越界检查两行可替换上述两行，因通过插入排序可避免
        // while (comp(data[++i], pivot));
        // while (comp(pivot, data[--j]));
        if (i >= j) break;
        std::swap(data[i], data[j]);
    }

    // 将枢轴放到正确位置
    std::swap(data[i], data[right - 1]);

    // 尾递归优化：先处理较小的子数组
    if (i - left < right - i) {
        hoare_quick_sort(data, left, i - 1, comp);
        hoare_quick_sort(data, i + 1, right, comp);
    }
    else {
        hoare_quick_sort(data, i + 1, right, comp);
        hoare_quick_sort(data, left, i - 1, comp);
    }
}

template<typename T>
void hoare_quick_sort(T* data, int left, int right)
{
    return hoare_quick_sort(data, left, right, std::less<T>());
}
template<typename T>
void three_way_quick_sort(T* data, int left, int right) {
    if (left >= right) return;

    const int size = right - left + 1;
    if (size <= 32) {
        std::sort(data + left, data + right + 1);
        return;
    }

    // 三数取中选择枢轴
    int mid = left + (right - left) / 2;
    if (data[mid] < data[left]) std::swap(data[left], data[mid]);
    if (data[right] < data[left]) std::swap(data[left], data[right]);
    if (data[right] < data[mid]) std::swap(data[mid], data[right]);

    T pivot = data[mid];

    // 三路分区
    int lt = left;      // data[left..lt-1] < pivot
    int gt = right;     // data[gt+1..right] > pivot
    int i = left;       // data[lt..i-1] == pivot

    while (i <= gt) {
        if (data[i] < pivot) {
            std::swap(data[lt], data[i]);
            ++lt;
            ++i;
        }
        else if (data[i] > pivot) {
            std::swap(data[i], data[gt]);
            --gt;
        }
        else {
            ++i;
        }
    }

    // 递归排序小于和大于区域
    three_way_quick_sort(data, left, lt - 1);
    three_way_quick_sort(data, gt + 1, right);
}

template<typename T>
void iterative_quick_sort(T* data, int left, int right) {
    struct Range {
        int left, right;
        Range(int l = 0, int r = 0) : left(l), right(r) {}
    };

    std::stack<Range> stk;
    stk.push(Range(left, right));

    while (!stk.empty()) {
        Range range = stk.top();
        stk.pop();
        int l = range.left, r = range.right;

        if (l >= r) continue;

        const int size = r - l + 1;
        if (size <= 32) {
            std::sort(data + l, data + r + 1);
            continue;
        }

        // 分区操作
        int mid = l + (r - l) / 2;
        T pivot = data[mid];
        int i = l, j = r;

        while (i <= j) {
            while (data[i] < pivot) ++i;
            while (data[j] > pivot) --j;
            if (i <= j) {
                std::swap(data[i], data[j]);
                ++i;
                --j;
            }
        }

        // 将较大的区间先压栈，较小的后压栈（减少栈深度）
        if (l < j) stk.push(Range(l, j));
        if (i < r) stk.push(Range(i, r));
    }
}

template<typename T>
void iterative_quick_sort_optimized1(T* data, int left, int right) {
    struct Range {
        int left, right;
        Range(int l = 0, int r = 0) : left(l), right(r) {}
    };

    // 使用预分配数组替代std::stack减少动态内存分配
    const int MAX_STACK_SIZE = 64; // 足够处理2^64大小的数组
    Range stack[MAX_STACK_SIZE];
    int stack_top = -1;

    stack[++stack_top] = Range(left, right);

    while (stack_top >= 0) {
        Range range = stack[stack_top--];
        int l = range.left, r = range.right;

        if (l >= r) continue;

        const int size = r - l + 1;
        if (size <= 32) {
            std::sort(data + l, data + r + 1);
            continue;
        }

        // 优化1: 三数取中法选择更好的枢轴
        int mid = l + (r - l) / 2;
        if (data[mid] < data[l]) std::swap(data[l], data[mid]);
        if (data[r] < data[l]) std::swap(data[l], data[r]);
        if (data[r] < data[mid]) std::swap(data[mid], data[r]);

        T pivot = data[mid];
        std::swap(data[mid], data[r - 1]); // 将枢轴放到倒数第二个位置

        int i = l, j = r - 1;
        while (true) {
            do { ++i; } while (i < right && data[i] < pivot);
            do { --j; } while (j > left && pivot < data[j]);
            // 以下无越界检查两行可替换上述两行，因通过插入排序可避免
            // while (data[++i] < pivot);
            // while (data[--j] > pivot);
            if (i >= j) break;
            std::swap(data[i], data[j]);
        }
        std::swap(data[i], data[r - 1]);

        if (i - l < r - i) {
            stack[++stack_top] = Range(l, i - 1);
            stack[++stack_top] = Range(i + 1, r);
        }
        else {
            stack[++stack_top] = Range(i + 1, r);
            stack[++stack_top] = Range(l, i - 1);
        }
        
    }
}

template<typename T>
void iterative_quick_sort_optimized2(T* data, int left, int right) {
    struct Range {
        int left, right;
        Range(int l = 0, int r = 0) : left(l), right(r) {}
    };

    // 使用预分配数组替代std::stack减少动态内存分配
    const int MAX_STACK_SIZE = 64; // 足够处理2^64大小的数组
    Range stack[MAX_STACK_SIZE];
    int stack_top = -1;

    stack[++stack_top] = Range(left, right);

    while (stack_top >= 0) {
        Range range = stack[stack_top--];
        int l = range.left, r = range.right;

        if (l >= r) continue;

        const int size = r - l + 1;
        if (size <= 32) {
            std::sort(data + l, data + r + 1);
            continue;
        }

        // 优化1: 三数取中法选择更好的枢轴
        int mid = l + (r - l) / 2;
        if (data[mid] < data[l]) std::swap(data[l], data[mid]);
        if (data[r] < data[l]) std::swap(data[l], data[r]);
        if (data[r] < data[mid]) std::swap(data[mid], data[r]);

        T pivot = data[mid];
        // 优化2: 更高效的分区算法（类似Hoare分区）
        int i = l, j = r;
        while (i <= j) {
            while (data[i] < pivot) ++i;
            while (data[j] > pivot) --j;
            if (i <= j) {
                std::swap(data[i], data[j]);
                ++i;
                --j;
            }
        }

        // 优化3: 总是先处理较大的分区，减少栈深度
        if (j - l > r - i) {
            // 左分区较大，先压入右分区
            if (i < r) stack[++stack_top] = Range(i, r);
            if (l < j) stack[++stack_top] = Range(l, j);
        }
        else {
            // 右分区较大，先压入左分区
            if (l < j) stack[++stack_top] = Range(l, j);
            if (i < r) stack[++stack_top] = Range(i, r);
        }
    }
}

template<typename T>
void timsort(T* data, int left, int right) {
    TimSort<T>::sort(data + left, right - left + 1);
}

// PDQSort 使用  
template<typename T>
void pdqsort(T* data, int left, int right) {
    PDQSort<T>::sort(data, left, right);
}

class QuickSortBenchmark {
private:
    template<typename T>
    using SortFunction = void(*)(T*, int, int);

    struct TestResult {
        std::string name;
        double time_ms;
        bool correct;
    };

    template<typename T>
    static bool is_sorted(const T* data, int size) {
        for (int i = 1; i < size; ++i) {
            if (data[i] < data[i - 1]) {
                return false;
            }
        }
        return true;
    }

    template<typename T>
    static bool arrays_equal(const T* arr1, const T* arr2, int size) {
        for (int i = 0; i < size; ++i) {
            if (arr1[i] != arr2[i]) {
                return false;
            }
        }
        return true;
    }

    template<int N = 0, typename T>
    static void test_sort_function(SortFunction<T> sort_func, const std::string& name,
        std::vector<T>& test_data, std::vector<T>& original_data,
        std::vector<TestResult>& results) {
        // 恢复原始数据
        test_data = original_data;

        auto start = std::chrono::high_resolution_clock::now();
        if constexpr (N)
        {
            std::sort(test_data.begin(), test_data.end(), std::less<T>());
        }
        else {
            sort_func(test_data.data(), 0, static_cast<int>(test_data.size()) - 1);
        }
        auto end = std::chrono::high_resolution_clock::now();

        double time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        bool correct = is_sorted(test_data.data(), static_cast<int>(test_data.size()));

        results.push_back({ name, time_ms, correct });

        std::cout << name << ": " << time_ms << " ms, "
            << (correct ? "Correct" : "INCORRECT") << std::endl;
    }

public:
    template<typename T>
    static void run_benchmark(int data_size = 100000) {
        std::cout << "=== Quick Sort Benchmark (Data Size: " << data_size << ") ===" << std::endl;

        // 测试数据生成器
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<T> dist(1, data_size);

        // 生成四种测试数据集
        std::vector<std::vector<T>> test_cases(4);
        std::vector<std::string> case_names = {
            "Sorted (Ascending)",
            "Sorted (Descending)",
            "Partially Sorted",
            "Random"
        };

        // 1. 已排序（升序）
        test_cases[0].resize(data_size);
        for (int i = 0; i < data_size; ++i) {
            test_cases[0][i] = static_cast<T>(i + 1);
        }

        // 2. 已排序（降序）
        test_cases[1].resize(data_size);
        for (int i = 0; i < data_size; ++i) {
            test_cases[1][i] = static_cast<T>(data_size - i);
        }

        // 3. 部分有序（整体无序，但包含有序片段）
        test_cases[2].resize(data_size);
        for (int i = 0; i < data_size; ++i) {
            test_cases[2][i] = dist(gen);
        }
        // 在随机数据中插入一些有序片段
        int segment_size = data_size / 10;
        for (int i = 0; i < data_size; i += segment_size) {
            int end = std::min(i + segment_size, data_size);
            std::sort(test_cases[2].begin() + i, test_cases[2].begin() + end);
        }

        // 4. 完全乱序
        test_cases[3].resize(data_size);
        for (int i = 0; i < data_size; ++i) {
            test_cases[3][i] = dist(gen);
        }

        // 测试所有排序算法
        std::vector<SortFunction<T>> sort_functions = {
            hash_pivot_quick_sort<T>,
            lomuto_quick_sort<T>,
            hoare_quick_sort<T>,
            three_way_quick_sort<T>,
            iterative_quick_sort<T>,
            iterative_quick_sort_optimized1<T>,
            iterative_quick_sort_optimized2<T>,
            timsort<T>,
            pdqsort<T>
        };

        std::vector<std::string> function_names = {
            "Hash Pivot Quick Sort",
            "Lomuto Quick Sort",
            "Hoare Quick Sort",
            "Three Way Quick Sort",
            "Iterative Quick Sort",
            "Iterative Quick Sort (Optimized 1)",
            "Iterative Quick Sort (Optimized 2)",
            "Timsort",
            "PDQSort"
        };

        for (int case_idx = 0; case_idx < 4; ++case_idx) {
            std::cout << "\n--- Testing: " << case_names[case_idx] << " ---" << std::endl;

            std::vector<T> original_data = test_cases[case_idx];
            std::vector<T> test_data;
            std::vector<TestResult> results;

            for (size_t i = 0; i < sort_functions.size(); ++i) {
                test_sort_function(sort_functions[i], function_names[i],
                    test_data, original_data, results);
            }
            test_sort_function<1>(sort_functions[0], "std::sort", test_data, original_data, results);

            // 找出最快的算法
            auto fastest = std::min_element(results.begin(), results.end(),
                [](const TestResult& a, const TestResult& b) {
                    return a.time_ms < b.time_ms;
                });

            std::cout << "Fastest: " << fastest->name << " (" << fastest->time_ms << " ms)" << std::endl;
        }
    }
};

// USING:    QuickSortBenchmark::run_benchmark<int>(100000);


// Windows MSVC v19.latest
// 升序（部分有序升序）：Timsort
// 降序：Iterative Quick Sort (Optimized 2) 有序数据性能最好
// 随机：PDQSort
/*
=== Quick Sort Benchmark (Data Size: 100000) ===

--- Testing: Sorted (Ascending) ---
Hash Pivot Quick Sort: 1.4512 ms, Correct
Lomuto Quick Sort: 0.8826 ms, Correct
Hoare Quick Sort: 0.5506 ms, Correct
Three Way Quick Sort: 17.8027 ms, Correct
Iterative Quick Sort: 0.7913 ms, Correct
Iterative Quick Sort (Optimized 1): 0.7979 ms, Correct
Iterative Quick Sort (Optimized 2): 0.6569 ms, Correct
Timsort: 0.1018 ms, Correct
PDQSort: 3.0503 ms, Correct
std::sort: 1.1629 ms, Correct
Fastest: Timsort (0.1018 ms)

--- Testing: Sorted (Descending) ---
Hash Pivot Quick Sort: 1.5681 ms, Correct
Lomuto Quick Sort: 1.4448 ms, Correct
Hoare Quick Sort: 1.0308 ms, Correct
Three Way Quick Sort: 13.4273 ms, Correct
Iterative Quick Sort: 0.6526 ms, Correct
Iterative Quick Sort (Optimized 1): 1.0599 ms, Correct
Iterative Quick Sort (Optimized 2): 0.6117 ms, Correct
Timsort: 1.9286 ms, Correct
PDQSort: 2.9562 ms, Correct
std::sort: 1.1613 ms, Correct
Fastest: Iterative Quick Sort (Optimized 2) (0.6117 ms)

--- Testing: Partially Sorted ---
Hash Pivot Quick Sort: 3.0282 ms, Correct
Lomuto Quick Sort: 4.2197 ms, Correct
Hoare Quick Sort: 2.5908 ms, Correct
Three Way Quick Sort: 3.8794 ms, Correct
Iterative Quick Sort: 3.0213 ms, Correct
Iterative Quick Sort (Optimized 1): 2.7891 ms, Correct
Iterative Quick Sort (Optimized 2): 2.8248 ms, Correct
Timsort: 2.0566 ms, Correct
PDQSort: 3.7713 ms, Correct
std::sort: 3.4708 ms, Correct
Fastest: Timsort (2.0566 ms)

--- Testing: Random ---
Hash Pivot Quick Sort: 5.5457 ms, Correct
Lomuto Quick Sort: 5.6394 ms, Correct
Hoare Quick Sort: 5.5317 ms, Correct
Three Way Quick Sort: 6.1913 ms, Correct
Iterative Quick Sort: 5.7493 ms, Correct
Iterative Quick Sort (Optimized 1): 5.2108 ms, Correct
Iterative Quick Sort (Optimized 2): 5.7935 ms, Correct
Timsort: 7.221 ms, Correct
PDQSort: 4.3957 ms, Correct
Fastest: PDQSort (4.3957 ms)
*/

/* GCC 15.2.0 C++23
* 部分有序升序：Timsort
* 降序、随机：std::sort
=== Quick Sort Benchmark (Data Size: 100000) ===

--- Testing: Sorted (Ascending) ---
Hash Pivot Quick Sort: 1.08409 ms, Correct
Lomuto Quick Sort: 1.02889 ms, Correct
Hoare Quick Sort: 1.00033 ms, Correct
Three Way Quick Sort: 12.3577 ms, Correct
Iterative Quick Sort: 0.646198 ms, Correct
Iterative Quick Sort (Optimized 1): 0.771409 ms, Correct
Iterative Quick Sort (Optimized 2): 0.587493 ms, Correct
Timsort: 0.199254 ms, Correct
PDQSort: 4.58019 ms, Correct
std::sort: 0.784269 ms, Correct
Fastest: Timsort (0.199254 ms)

--- Testing: Sorted (Descending) ---
Hash Pivot Quick Sort: 1.11581 ms, Correct
Lomuto Quick Sort: 1.31842 ms, Correct
Hoare Quick Sort: 1.36639 ms, Correct
Three Way Quick Sort: 8.83345 ms, Correct
Iterative Quick Sort: 0.651445 ms, Correct
Iterative Quick Sort (Optimized 1): 1.15953 ms, Correct
Iterative Quick Sort (Optimized 2): 0.602317 ms, Correct
Timsort: 1.27271 ms, Correct
PDQSort: 5.45043 ms, Correct
std::sort: 0.560567 ms, Correct
Fastest: std::sort (0.560567 ms)

--- Testing: Partially Sorted ---
Hash Pivot Quick Sort: 3.42166 ms, Correct
Lomuto Quick Sort: 4.36713 ms, Correct
Hoare Quick Sort: 3.5377 ms, Correct
Three Way Quick Sort: 3.99262 ms, Correct
Iterative Quick Sort: 3.52421 ms, Correct
Iterative Quick Sort (Optimized 1): 3.3393 ms, Correct
Iterative Quick Sort (Optimized 2): 3.32955 ms, Correct
Timsort: 2.04555 ms, Correct
PDQSort: 7.65374 ms, Correct
std::sort: 3.20401 ms, Correct
Fastest: Timsort (2.04555 ms)

--- Testing: Random ---
Hash Pivot Quick Sort: 6.79523 ms, Correct
Lomuto Quick Sort: 6.56841 ms, Correct
Hoare Quick Sort: 6.45855 ms, Correct
Three Way Quick Sort: 7.55213 ms, Correct
Iterative Quick Sort: 6.7526 ms, Correct
Iterative Quick Sort (Optimized 1): 6.42773 ms, Correct
Iterative Quick Sort (Optimized 2): 6.79702 ms, Correct
Timsort: 7.37144 ms, Correct
PDQSort: 9.59535 ms, Correct
std::sort: 6.29562 ms, Correct
Fastest: std::sort (6.29562 ms)
*/
/*
通过分析代码和两个平台的测试结果，总结各个排序算法的性能特点：

## 🏆 综合性能排名

### **Windows MSVC 环境：**
1. **PDQSort** - 随机数据最快
2. **Timsort** - 有序/部分有序数据最快
3. **Iterative Quick Sort (Optimized 2)** - 降序数据最快

### **GCC 环境：**
1. **std::sort** - 降序和随机数据最快
2. **Timsort** - 有序/部分有序数据最快
3. **Iterative Quick Sort (Optimized 2)** - 表现稳定

## 📊 时间复杂度分析

| 算法 | 最佳 | 平均 | 最差 | 稳定性 |
|------|------|------|------|--------|
| QuickSort 变体 | O(n log n) | O(n log n) | O(n²) | 不稳定 |
| Timsort | O(n) | O(n log n) | O(n log n) | 稳定 |
| PDQSort | O(n) | O(n log n) | O(n log n) | 不稳定 |
| std::sort | O(n log n) | O(n log n) | O(n log n) | 不稳定 |

## 💾 空间复杂度分析

| 算法 | 空间复杂度 | 特点 |
|------|------------|------|
| 递归快排 | O(log n) | 递归栈深度 |
| 迭代快排 | O(log n) | 显式栈管理 |
| Timsort | O(n) | 需要临时数组合并 |
| PDQSort | O(log n) | 递归但深度有限 |

## 🎯 最佳使用场景

### **Timsort**
- ✅ **最适合**：已排序、部分有序、反向有序数据
- ✅ **优势**：利用数据已有顺序，现实世界数据表现优秀
- ❌ **劣势**：随机数据较慢，需要额外O(n)空间
- 🎯 **用例**：几乎有序的数据、时间序列、日志数据

### **PDQSort**
- ✅ **最适合**：完全随机数据
- ✅ **优势**：防御性设计，防止快排退化，随机数据极快
- ❌ **劣势**：有序数据性能下降
- 🎯 **用例**：完全随机数据、通用排序

### **迭代快排优化版**
- ✅ **最适合**：降序数据、内存受限环境
- ✅ **优势**：避免递归开销，栈空间可控
- ❌ **劣势**：实现复杂，常数因子较大
- 🎯 **用例**：大数据集、嵌入式系统

### **std::sort**
- ✅ **最适合**：GCC环境下通用排序
- ✅ **优势**：编译器高度优化，综合性能均衡
- ❌ **劣势**：平台依赖性较强
- 🎯 **用例**：跨平台兼容性要求不高的场景

## 🔍 关键发现

1. **编译器差异显著**：MSVC和GCC的优化策略不同，导致算法表现差异
2. **数据特征决定性能**：没有"万能"算法，数据分布决定最佳选择
3. **现代算法优势**：PDQSort和Timsort在特定场景表现突出
4. **迭代vs递归**：迭代实现在某些场景有优势，但实现更复杂

## 🏆 综合推荐

**对于通用场景：**
```cpp
// 如果使用GCC：优先使用std::sort
std::sort(data, data + n);

// 如果使用MSVC且数据随机：使用PDQSort
PDQSort::sort(data, n);

// 如果数据部分有序：使用Timsort
TimSort::sort(data, n);
```

**最佳实践：**
- 小数组(≤32)：直接使用插入排序或std::sort
- 已知数据分布：选择针对性算法
- 未知数据：PDQSort或std::sort提供良好保障
- 内存敏感：选择迭代快排变体

*/

/* 
* 详细展开多少次，性能对比请查看 ../sort/benchmark_parallel_sort.hpp
* 测试使用 GCC 15.2.0  -std=c++20 -O3
class Solution {
private:
    template<typename BiIter, typename Compare>
    void insert_sort(BiIter start, BiIter end, Compare comp)
    {
#pragma GCC unroll 8
        for (auto current = std::next(start); current != end; ++current)
        {
            auto value = std::move(*current);
            auto hole = current;

            auto prev = hole;

            while (prev != start && comp(value, *std::prev(prev)))
            {
                --prev;
                *hole = std::move(*prev);
                hole = prev;
            }
            *hole = std::move(value);
        }
    }

    template<typename RanIter, typename Compare>
    void hoare_quick_sort(RanIter start, RanIter end, Compare comp)
    {
        const auto size = std::distance(start, end);
        if (size <= 32) {
            insert_sort(start, end, comp);
            return;
        }

        // 三数取中法选择枢轴, 确保 left ≤ mid ≤ right, 防止有序情况分布不均匀
        auto left = start, right = std::prev(end);
        auto mid = start + ((right - left) >> 1);

        if (comp(*mid, *left)) std::iter_swap(mid, left);
        if (comp(*right, *left)) std::iter_swap(right, left);
        if (comp(*right, *mid)) std::iter_swap(right, mid);

        auto pivot_val = *mid;
        // 优化分区循环的边界检查，如果不移动枢轴 需要额外处理枢轴位置 要跳过枢轴本身
        auto pivot_pos = std::prev(right); // 倒数第二个位置
        std::iter_swap(mid, pivot_pos); // 将枢轴放到倒数第二个位置

        // left(i) ≤ pivot_pos(j) ≤ right, 已有序情况下，我们从i的next，j的prev开始
        auto i = left, j = pivot_pos;
        while (true) {
            do { ++i; } while (i < right && comp(*i, pivot_val)); // 找比枢轴大
            do { --j; } while (j > left && comp(pivot_val, *j)); // 找比枢轴小
            if (i >= j) break;
            std::iter_swap(i, j);
        }

        // 将枢轴放到正确位置
        std::iter_swap(i, pivot_pos); // 此时i的值是枢轴值

        // 尾递归优化：先处理较小的子数组，减少递归深度，防止栈溢出
        if (std::distance(left, i) < std::distance(i, right)) {
            hoare_quick_sort(left, i, comp);
            hoare_quick_sort(std::next(i), end, comp); // 注意end是开区间
        }
        else {
            hoare_quick_sort(std::next(i), end, comp);
            hoare_quick_sort(left, i, comp);
        }
    }
public:
    std::vector<int> sortArray(std::vector<int>& nums) {
        hoare_quick_sort(nums.begin(), nums.end(), std::less<>{});
        return nums;
    }
};
*/