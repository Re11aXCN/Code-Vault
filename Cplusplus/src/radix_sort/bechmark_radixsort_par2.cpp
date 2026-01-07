// 测试有符号整数排序的基准测试，最后一趟分开单独处理符号位
// V1不分开，V2分开
// 测试结果：
// * V2在32位数据上优势明显：时间复杂度常数因子更小
// * 64位数据两者相当：没有明显的优胜者
#pragma once
#include <benchmark/benchmark.h>
#include <type_traits>
#include <vector>
#include <array>
#include <iterator>
#include <algorithm>
#include <bit>
#include <execution>
#include <omp.h>
#include "../stdex/profiling/generator.hpp"

template<typename T>
struct identity_key_extractor {
    template<typename U>
    constexpr auto operator()(U&& value) const noexcept
        -> std::enable_if_t<std::is_same_v<std::decay_t<U>, T>, T>
    {
        return std::forward<U>(value);
    }
};
template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v1(std::execution::parallel_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(_Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool> && sizeof(Key_t) >= 4,
        "Key type must be an arithmetic type (not bool) and at least 4 bytes. If you want to sort 8bits or 16bits, please using sequential_execution_policy.");

    using Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<Key_t>,
        std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>,
        Key_t>
    >;

    Value_t* _Ptr = &*_First;
    std::size_t _Size = std::distance(_First, _Last);
    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
    constexpr std::uint8_t _Shift = _Bucket_size == 256U ? 3 : 4;
    constexpr std::uint16_t _Mask = _Bucket_size - 1;
    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;

    const std::int32_t _Hardware_concurrency = omp_get_num_procs();

    // 优化：动态调整线程数，避免过小的数据块
    const std::size_t _Min_chunk_size = 1024;
    const std::int32_t _Actual_threads = std::max(1,
        std::min(_Hardware_concurrency, static_cast<std::int32_t>(_Size / _Min_chunk_size)));

    const std::size_t _Chunk = (_Size + _Actual_threads - 1) / _Actual_threads;

    // 优化：合并本地桶计数和偏移量数组，减少内存分配
    // 布局: [thread0_bucket0, thread0_offset0, thread1_bucket0, thread1_offset0, ...]
    std::vector<std::size_t> _Local_data(_Actual_threads * _Bucket_size * 2);

    // 辅助函数：获取线程的桶计数指针
    auto _Func_get_local_buckets = [&](int _Thread_id) -> std::size_t* {
        return _Local_data.data() + _Thread_id * _Bucket_size * 2;
        };

    // 辅助函数：获取线程的偏移量指针
    auto _Func_get_local_offsets = [&](int _Thread_id) -> std::size_t* {
        return _Local_data.data() + _Thread_id * _Bucket_size * 2 + _Bucket_size;
        };

    // 优化：使用固定大小的全局前缀数组，避免动态分配
    std::array<std::size_t, _Bucket_size> _Global_prefix;

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif
    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
        // Phase 1: 并行计数阶段
        {
            // 重置本地计数
            std::fill(_Local_data.begin(), _Local_data.end(), 0);

#pragma omp parallel num_threads(_Actual_threads)
            {
                const int _Thread_id = omp_get_thread_num();
                std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread_id);

                // 计算当前线程处理的区间
                const std::size_t _Start_idx = _Thread_id * _Chunk;
                const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                // 本地计数
                for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                    Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                    if constexpr (std::is_signed_v<Key_t>) {
                        if constexpr (std::is_floating_point_v<Key_t>) {
                            if constexpr (sizeof(Key_t) == 4)
                                _Unsigned_value ^= ((_Unsigned_value >> 31) == 0) ? 0x8000'0000U : 0xFFFF'FFFFU;
                            else if constexpr (sizeof(Key_t) == 8)
                                _Unsigned_value ^= ((_Unsigned_value >> 63) == 0) ? 0x8000'0000'0000'0000ULL : 0xFFFF'FFFF'FFFF'FFFFULL;
                        }
                        else {
                            if constexpr (sizeof(Key_t) == 1) _Unsigned_value ^= 0x80U;
                            else if constexpr (sizeof(Key_t) == 2) _Unsigned_value ^= 0x8000U;
                            else if constexpr (sizeof(Key_t) == 4) _Unsigned_value ^= 0x8000'0000U;
                            else if constexpr (sizeof(Key_t) == 8) _Unsigned_value ^= 0x8000'0000'0000'0000ULL;
                        }
                    }

                    std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

                    if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                    ++_Local_buckets[_Byte_idx];
                }
            }
        }

        // Phase 2: 优化的前缀和计算
        {
            // 步骤2.1: 全局归约 - 优化内存访问模式
            std::fill(_Global_prefix.begin(), _Global_prefix.end(), 0);

            for (std::int32_t _Thread = 0; _Thread < _Actual_threads; ++_Thread) {
                std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread);
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
                    _Global_prefix[_Bucket] += _Local_buckets[_Bucket];
                }
            }

            // 步骤2.2: 计算全局前缀和
            std::size_t _Running_sum = 0;
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
                std::size_t _Current_count = _Global_prefix[_Bucket];
                _Global_prefix[_Bucket] = _Running_sum;
                _Running_sum += _Current_count;
            }

            // 步骤2.3: 计算本地偏移量 - 单次遍历完成
            for (std::int32_t _Thread = 0; _Thread < _Actual_threads; ++_Thread) {
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
                std::size_t* __restrict _Local_buckets = _Func_get_local_buckets(_Thread);
                std::size_t* __restrict _Local_offsets = _Func_get_local_offsets(_Thread);
#else
                std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread);
                std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread);
#endif
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
                    _Local_offsets[_Bucket] = _Global_prefix[_Bucket];
                    _Global_prefix[_Bucket] += _Local_buckets[_Bucket];
                }
            }
        }

        // Phase 3: 并行散射阶段
        {
#pragma omp parallel num_threads(_Actual_threads)
            {
                const int _Thread_id = omp_get_thread_num();
                std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread_id);

                // 计算当前线程处理的区间
                const std::size_t _Start_idx = _Thread_id * _Chunk;
                const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                // 散射元素到正确位置
                for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                    auto _Value = std::move(_Start[_Idx]);
                    Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                    if constexpr (std::is_signed_v<Key_t>) {
                        if constexpr (std::is_floating_point_v<Key_t>) {
                            if constexpr (sizeof(Key_t) == 4)
                                _Unsigned_value ^= ((_Unsigned_value >> 31) == 0) ? 0x8000'0000U : 0xFFFF'FFFFU;
                            else if constexpr (sizeof(Key_t) == 8)
                                _Unsigned_value ^= ((_Unsigned_value >> 63) == 0) ? 0x8000'0000'0000'0000ULL : 0xFFFF'FFFF'FFFF'FFFFULL;
                        }
                        else {
                            if constexpr (sizeof(Key_t) == 1) _Unsigned_value ^= 0x80U;
                            else if constexpr (sizeof(Key_t) == 2) _Unsigned_value ^= 0x8000U;
                            else if constexpr (sizeof(Key_t) == 4) _Unsigned_value ^= 0x8000'0000U;
                            else if constexpr (sizeof(Key_t) == 8) _Unsigned_value ^= 0x8000'0000'0000'0000ULL;
                        }
                    }

                    std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

                    if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                    _End[_Local_offsets[_Byte_idx]++] = std::move(_Value);
                }
            }
        }

        std::swap(_Start, _End);
    }

    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v2(std::execution::parallel_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(_Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool> && sizeof(Key_t) >= 4,
        "Key type must be an arithmetic type (not bool) and at least 4 bytes. If you want to sort 8bits or 16bits, please using sequential_execution_policy.");

    using Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<Key_t>,
        std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>,
        Key_t>
    >;

    Value_t* _Ptr = &*_First;
    std::size_t _Size = std::distance(_First, _Last);
    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
    constexpr std::uint8_t _Shift = _Bucket_size == 256U ? 3 : 4;
    constexpr std::uint16_t _Mask = _Bucket_size - 1;
    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;

    const std::int32_t _Hardware_concurrency = omp_get_num_procs();

    // 优化：动态调整线程数，避免过小的数据块
    const std::size_t _Min_chunk_size = 1024;
    const std::int32_t _Actual_threads = std::max(1,
        std::min(_Hardware_concurrency, static_cast<std::int32_t>(_Size / _Min_chunk_size)));

    const std::size_t _Chunk = (_Size + _Actual_threads - 1) / _Actual_threads;

    // 优化：合并本地桶计数和偏移量数组，减少内存分配
    // 布局: [thread0_bucket0, thread0_offset0, thread1_bucket0, thread1_offset0, ...]
    std::vector<std::size_t> _Local_data(_Actual_threads * _Bucket_size * 2);

    // 辅助函数：获取线程的桶计数指针
    auto _Func_get_local_buckets = [&](int _Thread_id) -> std::size_t* {
        return _Local_data.data() + _Thread_id * _Bucket_size * 2;
        };

    // 辅助函数：获取线程的偏移量指针
    auto _Func_get_local_offsets = [&](int _Thread_id) -> std::size_t* {
        return _Local_data.data() + _Thread_id * _Bucket_size * 2 + _Bucket_size;
        };

    // 优化：使用固定大小的全局前缀数组，避免动态分配
    std::array<std::size_t, _Bucket_size> _Global_prefix;

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif
    for (std::uint8_t _Pass = 0; _Pass < _Passes - 1; ++_Pass) {
        // Phase 1: 并行计数阶段
        {
            // 重置本地计数
            std::fill(_Local_data.begin(), _Local_data.end(), 0);

#pragma omp parallel num_threads(_Actual_threads)
            {
                const int _Thread_id = omp_get_thread_num();
                std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread_id);

                // 计算当前线程处理的区间
                const std::size_t _Start_idx = _Thread_id * _Chunk;
                const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                // 本地计数
                for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                    Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= ((_Unsigned_value >> 31) == 0) ? 0x8000'0000U : 0xFFFF'FFFFU;
                        else if constexpr (sizeof(Key_t) == 8)
                            _Unsigned_value ^= ((_Unsigned_value >> 63) == 0) ? 0x8000'0000'0000'0000ULL : 0xFFFF'FFFF'FFFF'FFFFULL;
                    }

                    std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

                    if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                    ++_Local_buckets[_Byte_idx];
                }
            }
        }

        // Phase 2: 优化的前缀和计算
        {
            // 步骤2.1: 全局归约 - 优化内存访问模式
            std::fill(_Global_prefix.begin(), _Global_prefix.end(), 0);

            for (std::int32_t _Thread = 0; _Thread < _Actual_threads; ++_Thread) {
                std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread);
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
                    _Global_prefix[_Bucket] += _Local_buckets[_Bucket];
                }
            }

            // 步骤2.2: 计算全局前缀和
            std::size_t _Running_sum = 0;
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
                std::size_t _Current_count = _Global_prefix[_Bucket];
                _Global_prefix[_Bucket] = _Running_sum;
                _Running_sum += _Current_count;
            }

            // 步骤2.3: 计算本地偏移量 - 单次遍历完成
            for (std::int32_t _Thread = 0; _Thread < _Actual_threads; ++_Thread) {
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
                std::size_t* __restrict _Local_buckets = _Func_get_local_buckets(_Thread);
                std::size_t* __restrict _Local_offsets = _Func_get_local_offsets(_Thread);
#else
                std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread);
                std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread);
#endif
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
                    _Local_offsets[_Bucket] = _Global_prefix[_Bucket];
                    _Global_prefix[_Bucket] += _Local_buckets[_Bucket];
                }
            }
        }

        // Phase 3: 并行散射阶段
        {
#pragma omp parallel num_threads(_Actual_threads)
            {
                const int _Thread_id = omp_get_thread_num();
                std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread_id);

                // 计算当前线程处理的区间
                const std::size_t _Start_idx = _Thread_id * _Chunk;
                const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                // 散射元素到正确位置
                for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                    auto _Value = std::move(_Start[_Idx]);
                    Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= ((_Unsigned_value >> 31) == 0) ? 0x8000'0000U : 0xFFFF'FFFFU;
                        else if constexpr (sizeof(Key_t) == 8)
                            _Unsigned_value ^= ((_Unsigned_value >> 63) == 0) ? 0x8000'0000'0000'0000ULL : 0xFFFF'FFFF'FFFF'FFFFULL;
                    }

                    std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

                    if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                    _End[_Local_offsets[_Byte_idx]++] = std::move(_Value);
                }
            }
        }

        std::swap(_Start, _End);
    }
    //////////////////////////////////////////
    // Phase 1: 并行计数阶段
    {
        // 重置本地计数
        std::fill(_Local_data.begin(), _Local_data.end(), 0);

#pragma omp parallel num_threads(_Actual_threads)
        {
            const int _Thread_id = omp_get_thread_num();
            std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread_id);

            // 计算当前线程处理的区间
            const std::size_t _Start_idx = _Thread_id * _Chunk;
            const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            // 本地计数
            for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= ((_Unsigned_value >> 31) == 0) ? 0x8000'0000U : 0xFFFF'FFFFU;
                        else if constexpr (sizeof(Key_t) == 8)
                            _Unsigned_value ^= ((_Unsigned_value >> 63) == 0) ? 0x8000'0000'0000'0000ULL : 0xFFFF'FFFF'FFFF'FFFFULL;
                    }
                    else {
                        if constexpr (sizeof(Key_t) == 1) _Unsigned_value ^= 0x80U;
                        else if constexpr (sizeof(Key_t) == 2) _Unsigned_value ^= 0x8000U;
                        else if constexpr (sizeof(Key_t) == 4) _Unsigned_value ^= 0x8000'0000U;
                        else if constexpr (sizeof(Key_t) == 8) _Unsigned_value ^= 0x8000'0000'0000'0000ULL;
                    }
                }

                std::uint16_t _Byte_idx = (_Unsigned_value >> ((_Passes - 1) << _Shift)) & _Mask;

                if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                ++_Local_buckets[_Byte_idx];
            }
        }
    }

    // Phase 2: 优化的前缀和计算
    {
        // 步骤2.1: 全局归约 - 优化内存访问模式
        std::fill(_Global_prefix.begin(), _Global_prefix.end(), 0);

        for (std::int32_t _Thread = 0; _Thread < _Actual_threads; ++_Thread) {
            std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread);
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
                _Global_prefix[_Bucket] += _Local_buckets[_Bucket];
            }
        }

        // 步骤2.2: 计算全局前缀和
        std::size_t _Running_sum = 0;
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
            std::size_t _Current_count = _Global_prefix[_Bucket];
            _Global_prefix[_Bucket] = _Running_sum;
            _Running_sum += _Current_count;
        }

        // 步骤2.3: 计算本地偏移量 - 单次遍历完成
        for (std::int32_t _Thread = 0; _Thread < _Actual_threads; ++_Thread) {
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
            std::size_t* __restrict _Local_buckets = _Func_get_local_buckets(_Thread);
            std::size_t* __restrict _Local_offsets = _Func_get_local_offsets(_Thread);
#else
            std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread);
            std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread);
#endif
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
                _Local_offsets[_Bucket] = _Global_prefix[_Bucket];
                _Global_prefix[_Bucket] += _Local_buckets[_Bucket];
            }
        }
    }

    // Phase 3: 并行散射阶段
    {
#pragma omp parallel num_threads(_Actual_threads)
        {
            const int _Thread_id = omp_get_thread_num();
            std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread_id);

            // 计算当前线程处理的区间
            const std::size_t _Start_idx = _Thread_id * _Chunk;
            const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            // 散射元素到正确位置
            for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                auto _Value = std::move(_Start[_Idx]);
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= ((_Unsigned_value >> 31) == 0) ? 0x8000'0000U : 0xFFFF'FFFFU;
                        else if constexpr (sizeof(Key_t) == 8)
                            _Unsigned_value ^= ((_Unsigned_value >> 63) == 0) ? 0x8000'0000'0000'0000ULL : 0xFFFF'FFFF'FFFF'FFFFULL;
                    }
                    else {
                        if constexpr (sizeof(Key_t) == 1) _Unsigned_value ^= 0x80U;
                        else if constexpr (sizeof(Key_t) == 2) _Unsigned_value ^= 0x8000U;
                        else if constexpr (sizeof(Key_t) == 4) _Unsigned_value ^= 0x8000'0000U;
                        else if constexpr (sizeof(Key_t) == 8) _Unsigned_value ^= 0x8000'0000'0000'0000ULL;
                    }
                }

                std::uint16_t _Byte_idx = (_Unsigned_value >> ((_Passes - 1) << _Shift)) & _Mask;

                if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                _End[_Local_offsets[_Byte_idx]++] = std::move(_Value);
            }
        }
    }

    std::swap(_Start, _End);
    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}

// 模板化的并行基准测试函数
template<typename T, typename SortFunction, bool Ascending = true>
void BM_ParallelSort_Typed_Helper(benchmark::State& state) {
    auto size = state.range(0);

    // 根据类型选择适当的范围
    T min_val, max_val;
    if constexpr (std::is_same_v<T, int32_t>) {
        min_val = std::numeric_limits<T>::min();
        max_val = std::numeric_limits<T>::max();
    }
    else if constexpr (std::is_same_v<T, uint32_t>) {
        min_val = std::numeric_limits<T>::min();
        max_val = std::numeric_limits<T>::max();
    }
    else if constexpr (std::is_same_v<T, int64_t>) {
        min_val = std::numeric_limits<T>::min();
        max_val = std::numeric_limits<T>::max();
    }
    else if constexpr (std::is_same_v<T, uint64_t>) {
        min_val = std::numeric_limits<T>::min();
        max_val = std::numeric_limits<T>::max();
    }
    else if constexpr (std::is_same_v<T, float>) {
        min_val = (float)std::numeric_limits<int32_t>::min();
        max_val = (float)std::numeric_limits<int32_t>::max();
    }
    else if constexpr (std::is_same_v<T, double>) {
        min_val = (double)std::numeric_limits<int64_t>::min();
        max_val = (double)std::numeric_limits<int64_t>::max();
    }
    else {
        min_val = std::numeric_limits<T>::min();
        max_val = std::numeric_limits<T>::max();
    }

    const auto& test_data = stdex::generate_random<T>(size, min_val, max_val);

    for (auto _ : state) {
        state.PauseTiming();
        auto temp = test_data;
        state.ResumeTiming();

        using Iter_t = decltype(temp.begin());
        using Value_t = T;

        if constexpr (Ascending) {
            SortFunction{}(temp.begin(), temp.end(), std::less<>{},
                identity_key_extractor<Value_t>{});
        }
        else {
            SortFunction{}(temp.begin(), temp.end(), std::greater<>{},
                identity_key_extractor<Value_t>{});
        }

        // 验证排序正确性（只在调试时开启）
#ifdef DEBUG_SORT
        if (!is_sorted_correctly(temp, Ascending)) {
            state.SkipWithError("Sorting failed!");
        }
#endif

        benchmark::DoNotOptimize(temp.data());
    }

    state.SetBytesProcessed(state.iterations() * size * sizeof(T));
    state.SetComplexityN(size);
}

// 并行排序函数包装器
struct ParallelRadixSortV1Wrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        radix_sort_v1<Iter, Compare, Extractor, 256U>(
            std::execution::par, first, last, std::forward<Extractor>(extractor));
    }
};

struct ParallelRadixSortV2Wrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        radix_sort_v2<Iter, Compare, Extractor, 256U>(
            std::execution::par, first, last, std::forward<Extractor>(extractor));
    }
};

// 32位类型并行测试
BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int32_t, ParallelRadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int32_t, ParallelRadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, uint32_t, ParallelRadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, uint32_t, ParallelRadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int64_t, ParallelRadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int64_t, ParallelRadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, uint64_t, ParallelRadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, uint64_t, ParallelRadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, float, ParallelRadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, float, ParallelRadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, double, ParallelRadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, double, ParallelRadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_MAIN();


/*
-------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                                 Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------------------------------------------------------
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/1024          0.011 ms        0.011 ms        64000 bytes_per_second=347.826Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/2048          0.036 ms        0.035 ms        21333 bytes_per_second=222.219Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/4096          0.066 ms        0.067 ms        10000 bytes_per_second=232.558Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/8192          0.095 ms        0.090 ms         6400 bytes_per_second=345.946Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/16384         0.112 ms        0.107 ms         6400 bytes_per_second=581.818Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/32768         0.138 ms        0.141 ms         4978 bytes_per_second=884.978Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/65536         0.227 ms        0.200 ms         3200 bytes_per_second=1.21951Gi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/131072        0.596 ms        0.656 ms         1000 bytes_per_second=761.905Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/262144         1.04 ms        0.977 ms          640 bytes_per_second=1Gi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/524288         2.06 ms         1.99 ms          299 bytes_per_second=1007.16Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>/1048576        3.96 ms         4.19 ms          179 bytes_per_second=954.667Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>_BigO           3.82 N          3.95 N
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV1Wrapper, true>_RMS               7 %             8 %
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/1024          0.010 ms        0.010 ms        74667 bytes_per_second=397.165Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/2048          0.036 ms        0.036 ms        17920 bytes_per_second=218.537Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/4096          0.069 ms        0.070 ms        11200 bytes_per_second=224Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/8192          0.086 ms        0.087 ms         8960 bytes_per_second=358.4Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/16384         0.108 ms        0.105 ms         6400 bytes_per_second=595.349Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/32768         0.131 ms        0.129 ms         4978 bytes_per_second=971.317Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/65536         0.201 ms        0.209 ms         3446 bytes_per_second=1.17052Gi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/131072        0.571 ms        0.547 ms         1000 bytes_per_second=914.286Mi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/262144         1.03 ms        0.962 ms          747 bytes_per_second=1.01495Gi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/524288         1.89 ms         1.95 ms          280 bytes_per_second=1Gi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>/1048576        3.71 ms         3.82 ms          172 bytes_per_second=1.02381Gi/s
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>_BigO           3.58 N          3.66 N
BM_ParallelSort_Typed_Helper<int32_t, ParallelRadixSortV2Wrapper, true>_RMS               8 %             5 %
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/1024         0.010 ms        0.010 ms        64000 bytes_per_second=372.093Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/2048         0.039 ms        0.039 ms        17920 bytes_per_second=199.111Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/4096         0.068 ms        0.064 ms        11200 bytes_per_second=243.478Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/8192         0.086 ms        0.085 ms         8960 bytes_per_second=365.714Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/16384        0.111 ms        0.117 ms         6400 bytes_per_second=533.333Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/32768        0.134 ms        0.134 ms         5600 bytes_per_second=933.333Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/65536        0.209 ms        0.201 ms         3733 bytes_per_second=1.21517Gi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/131072       0.595 ms        0.609 ms         1000 bytes_per_second=820.513Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/262144       0.992 ms        0.920 ms          747 bytes_per_second=1.06108Gi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/524288        2.01 ms         2.00 ms          407 bytes_per_second=1001.85Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>/1048576       3.78 ms         3.91 ms          224 bytes_per_second=1Gi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>_BigO          3.67 N          3.74 N
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV1Wrapper, true>_RMS              8 %             7 %
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/1024         0.010 ms        0.010 ms        64000 bytes_per_second=400Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/2048         0.034 ms        0.035 ms        20364 bytes_per_second=226.267Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/4096         0.069 ms        0.070 ms        11200 bytes_per_second=224Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/8192         0.084 ms        0.086 ms         7467 bytes_per_second=364.244Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/16384        0.109 ms        0.110 ms         6400 bytes_per_second=568.889Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/32768        0.132 ms        0.128 ms         5600 bytes_per_second=973.913Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/65536        0.203 ms        0.181 ms         3446 bytes_per_second=1.34609Gi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/131072       0.569 ms        0.558 ms         1120 bytes_per_second=896Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/262144       0.980 ms         1.00 ms          640 bytes_per_second=999.024Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/524288        1.89 ms         2.01 ms          498 bytes_per_second=996Mi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>/1048576       3.62 ms         3.43 ms          187 bytes_per_second=1.14024Gi/s
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>_BigO          3.51 N          3.41 N
BM_ParallelSort_Typed_Helper<uint32_t, ParallelRadixSortV2Wrapper, true>_RMS              8 %            14 %
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/1024          0.022 ms        0.022 ms        32000 bytes_per_second=347.826Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/2048          0.066 ms        0.067 ms        11200 bytes_per_second=233.333Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/4096          0.111 ms        0.114 ms         5600 bytes_per_second=273.171Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/8192          0.142 ms        0.145 ms         5600 bytes_per_second=430.769Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/16384         0.177 ms        0.169 ms         4073 bytes_per_second=740.545Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/32768         0.258 ms        0.262 ms         2800 bytes_per_second=953.191Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/65536         0.632 ms        0.680 ms          896 bytes_per_second=735.179Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/131072         1.12 ms         1.12 ms          640 bytes_per_second=890.435Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/262144         2.11 ms         1.97 ms          373 bytes_per_second=1015.83Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/524288         4.32 ms         4.16 ms          154 bytes_per_second=961.561Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>/1048576        11.1 ms         10.9 ms           56 bytes_per_second=735.179Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>_BigO           0.51 NlgN       0.50 NlgN
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV1Wrapper, true>_RMS              16 %            17 %
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/1024          0.020 ms        0.020 ms        34462 bytes_per_second=391.614Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/2048          0.063 ms        0.066 ms        11200 bytes_per_second=238.298Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/4096          0.119 ms        0.122 ms         6400 bytes_per_second=256Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/8192          0.141 ms        0.141 ms         4978 bytes_per_second=442.489Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/16384         0.177 ms        0.176 ms         4073 bytes_per_second=708.348Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/32768         0.253 ms        0.243 ms         2635 bytes_per_second=1.00419Gi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/65536         0.628 ms        0.684 ms         1120 bytes_per_second=731.429Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/131072         1.12 ms         1.12 ms          640 bytes_per_second=890.435Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/262144         2.06 ms         2.09 ms          299 bytes_per_second=956.8Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/524288         4.31 ms         4.20 ms          160 bytes_per_second=952.558Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>/1048576        11.7 ms         12.0 ms           56 bytes_per_second=666.791Mi/s
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>_BigO           0.53 NlgN       0.54 NlgN
BM_ParallelSort_Typed_Helper<int64_t, ParallelRadixSortV2Wrapper, true>_RMS              19 %            23 %
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/1024         0.021 ms        0.021 ms        37333 bytes_per_second=373.33Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/2048         0.063 ms        0.063 ms        11200 bytes_per_second=248.889Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/4096         0.114 ms        0.105 ms         6400 bytes_per_second=297.674Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/8192         0.143 ms        0.141 ms         4978 bytes_per_second=442.489Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/16384        0.177 ms        0.167 ms         3733 bytes_per_second=746.6Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/32768        0.257 ms        0.261 ms         2635 bytes_per_second=958.182Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/65536        0.659 ms        0.609 ms         1000 bytes_per_second=820.513Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/131072        1.07 ms         1.00 ms          640 bytes_per_second=999.024Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/262144        2.08 ms         2.04 ms          345 bytes_per_second=981.333Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/524288        4.26 ms         4.39 ms          160 bytes_per_second=910.222Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>/1048576       11.4 ms         11.4 ms           56 bytes_per_second=699.317Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>_BigO          0.52 NlgN       0.52 NlgN
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV1Wrapper, true>_RMS             19 %            17 %
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/1024         0.020 ms        0.020 ms        34462 bytes_per_second=391.614Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/2048         0.062 ms        0.062 ms        13380 bytes_per_second=252.453Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/4096         0.110 ms        0.112 ms         5600 bytes_per_second=280Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/8192         0.142 ms        0.141 ms         4978 bytes_per_second=442.489Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/16384        0.174 ms        0.169 ms         4073 bytes_per_second=740.545Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/32768        0.251 ms        0.237 ms         2635 bytes_per_second=1.0293Gi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/65536        0.655 ms        0.609 ms         1000 bytes_per_second=820.513Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/131072        1.07 ms         1.09 ms          560 bytes_per_second=918.974Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/262144        2.03 ms         1.93 ms          373 bytes_per_second=1.01359Gi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/524288        4.09 ms         3.91 ms          172 bytes_per_second=1Gi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>/1048576       11.3 ms         11.6 ms           50 bytes_per_second=691.892Mi/s
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>_BigO          0.51 NlgN       0.52 NlgN
BM_ParallelSort_Typed_Helper<uint64_t, ParallelRadixSortV2Wrapper, true>_RMS             21 %            26 %
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/1024            0.012 ms        0.012 ms        56000 bytes_per_second=333.333Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/2048            0.031 ms        0.032 ms        23579 bytes_per_second=245.615Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/4096            0.058 ms        0.059 ms        11200 bytes_per_second=266.667Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/8192            0.076 ms        0.077 ms         8960 bytes_per_second=407.273Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/16384           0.101 ms        0.103 ms         6400 bytes_per_second=609.524Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/32768           0.137 ms        0.132 ms         4978 bytes_per_second=948.19Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/65536           0.220 ms        0.215 ms         3200 bytes_per_second=1.13636Gi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/131072          0.629 ms        0.562 ms         1000 bytes_per_second=888.889Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/262144           1.08 ms         1.00 ms          640 bytes_per_second=999.024Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/524288           2.08 ms         1.88 ms          448 bytes_per_second=1.03704Gi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>/1048576          4.08 ms         4.00 ms          172 bytes_per_second=1000.73Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>_BigO             3.93 N          3.78 N
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV1Wrapper, true>_RMS                 6 %             6 %
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/1024            0.012 ms        0.012 ms        56000 bytes_per_second=325.581Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/2048            0.032 ms        0.032 ms        22400 bytes_per_second=243.478Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/4096            0.058 ms        0.058 ms        10000 bytes_per_second=270.27Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/8192            0.076 ms        0.074 ms        11200 bytes_per_second=422.642Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/16384           0.099 ms        0.101 ms         8960 bytes_per_second=617.931Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/32768           0.133 ms        0.141 ms         4978 bytes_per_second=884.978Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/65536           0.214 ms        0.220 ms         3200 bytes_per_second=1.11111Gi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/131072          0.606 ms        0.641 ms         1000 bytes_per_second=780.488Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/262144           1.01 ms         1.10 ms          498 bytes_per_second=910.629Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/524288           1.96 ms         2.01 ms          373 bytes_per_second=994.667Mi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>/1048576          3.93 ms         3.75 ms          204 bytes_per_second=1.04082Gi/s
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>_BigO             3.76 N          3.67 N
BM_ParallelSort_Typed_Helper<float, ParallelRadixSortV2Wrapper, true>_RMS                 6 %            11 %
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/1024           0.024 ms        0.025 ms        29867 bytes_per_second=317.734Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/2048           0.059 ms        0.060 ms        11200 bytes_per_second=260.465Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/4096           0.104 ms        0.105 ms         7467 bytes_per_second=298.68Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/8192           0.140 ms        0.141 ms         4978 bytes_per_second=442.489Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/16384          0.182 ms        0.193 ms         3733 bytes_per_second=649.217Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/32768          0.273 ms        0.276 ms         2489 bytes_per_second=905.091Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/65536          0.705 ms        0.670 ms         1120 bytes_per_second=746.667Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/131072          1.15 ms         1.29 ms          640 bytes_per_second=772.83Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/262144          2.15 ms         2.13 ms          345 bytes_per_second=939.574Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/524288          4.49 ms         5.02 ms          112 bytes_per_second=796.444Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>/1048576         11.4 ms         11.9 ms           50 bytes_per_second=673.684Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>_BigO            0.53 NlgN       0.55 NlgN
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV1Wrapper, true>_RMS               15 %            12 %
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/1024           0.025 ms        0.025 ms        29867 bytes_per_second=317.734Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/2048           0.059 ms        0.059 ms        11200 bytes_per_second=266.667Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/4096           0.102 ms        0.103 ms         6400 bytes_per_second=304.762Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/8192           0.137 ms        0.138 ms         4978 bytes_per_second=452.545Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/16384          0.179 ms        0.184 ms         3733 bytes_per_second=678.727Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/32768          0.268 ms        0.276 ms         2489 bytes_per_second=905.091Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/65536          0.664 ms        0.656 ms         1120 bytes_per_second=762.553Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/131072          1.14 ms        0.977 ms          640 bytes_per_second=1Gi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/262144          2.20 ms         2.04 ms          299 bytes_per_second=981.333Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/524288          4.33 ms         3.75 ms          154 bytes_per_second=1.04054Gi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>/1048576         11.9 ms         11.4 ms           56 bytes_per_second=699.317Mi/s
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>_BigO            0.54 NlgN       0.51 NlgN
BM_ParallelSort_Typed_Helper<double, ParallelRadixSortV2Wrapper, true>_RMS               20 %            27 %

*/