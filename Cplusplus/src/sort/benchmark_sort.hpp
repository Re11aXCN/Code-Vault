#pragma once
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <execution>
#include <type_traits>
#include <limits>
#include <set>

#include <tbb/parallel_invoke.h>
#include <tbb/parallel_sort.h>

#include "utils/ticktock.h"
#include "utils/generator.hpp"

// i5-10200H CPU @ 2.40GHz
// VS2022 MSVC C++23
// 
// 非并行: 数量级是 1 << 13 时开始比标准库的std::sort快
// 并行: 数量级是 1 << 23 时开始比标准库的std::sort(std::execution::par)快
// 
// 什么时候并行比非并行快: std::sort和quick_sort都是 数量级是在 1 << 16 与 1 << 17 之间
// std::sort 接近于 1 << 16， quick_sort接近于1 << 17
// 
// 1 << 16数量级以内， quick_sort非并行略快于std::sort(std::execution::par)，差距在3%~20%，1 << 17就明显慢于并行
// 
// tbb::parallel_sort 数量级在1 << 17时和std::sort(std::execution::par)差不多，低于就比sort慢，高于就比sort快
//
// 1 << 10 希尔排序、quick_sort看数据的随机分布情况最优，低于1 << 10 quick_sort最优
// 1 << 11 std::sort最优
struct parallel_tag {};
struct sequential_tag {};
template <typename _Pa, typename _Ty, typename _Compare>
auto quick_sort([[maybe_unused]] _Pa&& para, _Ty* data, std::size_t size, _Compare comp) -> void
{
    if (size <= 32) {
        std::sort(data, data + size, std::less<_Ty>{}); // Insert_Sort
        return;
    }

    // 更好的随机化基准选择
    std::size_t mid = std::hash<std::size_t>{}(size);
    mid ^= std::hash<void*>{}(static_cast<void*>(data));
    mid %= size;
    std::swap(data[0], data[mid]);

    _Ty pivot = data[0];
    std::size_t left = 1, right = size - 1;

    // 正确的分区算法
    while (left <= right) {
        while (left <= right && comp(data[left], pivot)) ++left;
        while (left <= right && comp(pivot, data[right])) --right;
        if (left <= right) {
            std::swap(data[left], data[right]);
            ++left;
            --right;
        }
    }

    // 将基准放到正确位置
    std::swap(data[0], data[right]);

    // 递归排序
    if constexpr (std::is_same_v<std::decay_t<_Pa>, parallel_tag>) {
        tbb::parallel_invoke(
            [&] {
                quick_sort(para, data, right, comp);
            },
            [&] {
                quick_sort(para, data + right + 1, size - right - 1, comp);
            });
    }
    else {
        quick_sort(para, data, right, comp);
        quick_sort(para, data + right + 1, size - right - 1, comp);
    }
}

template <typename _Pa, typename _Ty>
auto quick_sort([[maybe_unused]] _Pa&& para, _Ty* data, std::size_t size) -> void
{
    quick_sort(std::forward<_Pa>(para), data, size, std::less<_Ty>{});
}

template <typename _Ty>
auto shell_sort(_Ty* data, std::size_t size) -> void
{
    std::size_t h = 1;
    while (h < size / 3) h = 3 * h + 1;
    while (h >= 1) {
        for (std::size_t i = h; i < size; ++i) {
            _Ty temp = data[i];
            std::size_t j;
            for (j = i; j >= h && data[j - h] > temp; j -= h) {
                data[j] = data[j - h];
            }
            data[j] = temp;
        }
        h /= 3;
    }
}

template <typename _Ty>
auto insertion_sort(_Ty* data, std::size_t size) -> void
{
    for (std::size_t i = 1; i < size; ++i) {
        _Ty temp = data[i];
        std::size_t j;
        for (j = i; j > 0 && data[j - 1] > temp; --j) {
            data[j] = data[j - 1];
        }
        data[j] = temp;
    }
}

static auto benchmark_sort() -> void
{
    auto arr = generate_vec_data(1 << 10, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max());
    auto arr2 = arr;
    auto arr3 = arr;
    auto arr4 = arr;
    auto arr5 = arr;
    TICK(quick_sort)
    quick_sort(sequential_tag{}, arr.data(), arr.size(), std::less<int32_t>{});
    TOCK(quick_sort)

    TICK(quick_sort_std_parallel)
    std::sort(std::execution::par, arr2.begin(), arr2.end());
    TOCK(quick_sort_std_parallel)

    TICK(shell_sort)
    shell_sort(arr3.data(), arr3.size());
    TOCK(shell_sort)

    // tbb有缓存，如果上述开启parallel_invoke, 并行sort那么这里速度将会非常快
    TICK(tbb_parallel_sort)
    tbb::parallel_sort(arr4.begin(), arr4.end());
    TOCK(tbb_parallel_sort)

    TICK(multiset_sort)
    std::multiset<int32_t> set(arr5.begin(), arr5.end());
    std::move(set.begin(), set.end(), arr5.data());
    TOCK(multiset_sort)
}