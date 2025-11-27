// 测试不同并行策略下的基数排序不同版本的性能
// v1 计算前缀和采取外层遍历桶，内层遍历核心
// v2 v1版本循环展开
// v3 计算前缀和采取内层遍历核心，外层遍历桶，  其次
// v4 v3版本循环展开
// v1~v4 都是都是参考seq版本同一思想策略实现
// 
// v5 三阶段方法（全局归约→全局前缀→本地偏移），最优版本

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
#include "tools/generator.hpp"

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

    std::int32_t _Hardware_concurrency = omp_get_num_procs();
    std::size_t _Chunk = (_Size + _Hardware_concurrency - 1) / _Hardware_concurrency;
    std::array<std::size_t, _Bucket_size> _Bucket_count;
    std::vector<std::size_t> _Processor_local_buckets(_Bucket_size * _Hardware_concurrency);

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif

    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::fill(_Plb_ptr, _Plb_ptr + _Bucket_size, 0);

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                ++_Plb_ptr[_Byte_idx];
            }
        }
        std::array<std::size_t, _Bucket_size> _Bucket_prefix{};

        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            for (std::size_t _Idx = 0; _Idx < _Bucket_size; ++_Idx) {
                std::size_t _Idx_local = _Bucket_size * _Core + _Idx;
                std::size_t _Temp_count_local = _Processor_local_buckets[_Idx_local];

                _Processor_local_buckets[_Idx_local] = _Bucket_prefix[_Idx];
                _Bucket_prefix[_Idx] += _Temp_count_local;
            }
        }

        std::exclusive_scan(_Bucket_prefix.begin(), _Bucket_prefix.end(), _Bucket_count.begin(), 0);

#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::transform(_Plb_ptr, _Plb_ptr + _Bucket_size, _Bucket_count.data(), _Plb_ptr, std::plus<>{});

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                auto _Value = std::move(_Start[_Idx]);
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                _End[_Plb_ptr[_Byte_idx]++] = _Value;
            }
        }

        // Swap buffer
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

    std::int32_t _Hardware_concurrency = omp_get_num_procs();
    std::size_t _Chunk = (_Size + _Hardware_concurrency - 1) / _Hardware_concurrency;
    std::array<std::size_t, _Bucket_size> _Bucket_count;
    std::vector<std::size_t> _Processor_local_buckets(_Bucket_size * _Hardware_concurrency);

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif

    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::fill(_Plb_ptr, _Plb_ptr + _Bucket_size, 0);

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                ++_Plb_ptr[_Byte_idx];
            }
        }

        std::array<std::size_t, _Bucket_size> _Bucket_prefix{};

        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = 0; _Idx < _Bucket_size; ++_Idx) {
                std::size_t _Idx_local = _Bucket_size * _Core + _Idx;
                std::size_t _Temp_count_local = _Processor_local_buckets[_Idx_local];

                _Processor_local_buckets[_Idx_local] = _Bucket_prefix[_Idx];
                _Bucket_prefix[_Idx] += _Temp_count_local;
            }
        }

        std::exclusive_scan(_Bucket_prefix.begin(), _Bucket_prefix.end(), _Bucket_count.begin(), 0);


#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::transform(_Plb_ptr, _Plb_ptr + _Bucket_size, _Bucket_count.data(), _Plb_ptr, std::plus<>{});

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                auto _Value = std::move(_Start[_Idx]);
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                _End[_Plb_ptr[_Byte_idx]++] = _Value;
            }
        }

        // Swap buffer
        std::swap(_Start, _End);
    }
    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v3(std::execution::parallel_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
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

    std::int32_t _Hardware_concurrency = omp_get_num_procs();
    std::size_t _Chunk = (_Size + _Hardware_concurrency - 1) / _Hardware_concurrency;
    std::array<std::size_t, _Bucket_size> _Bucket_count;
    std::vector<std::size_t> _Processor_local_buckets(_Bucket_size * _Hardware_concurrency);

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif

    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::fill(_Plb_ptr, _Plb_ptr + _Bucket_size, 0);

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                ++_Plb_ptr[_Byte_idx];
            }
        }
        std::size_t _Temp_sum{ 0 };
        for (std::size_t _Idx = 0; _Idx < _Bucket_size; ++_Idx) {
            std::size_t _Temp_sum_local{ 0 };
            for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
                std::size_t _Idx_local = _Bucket_size * _Core + _Idx;
                std::size_t _Temp_count_local = _Processor_local_buckets[_Idx_local];
                _Processor_local_buckets[_Idx_local] = _Temp_sum_local;
                _Temp_sum_local += _Temp_count_local;
            }
            _Bucket_count[_Idx] = _Temp_sum;
            _Temp_sum += _Temp_sum_local;
        }

#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::transform(_Plb_ptr, _Plb_ptr + _Bucket_size, _Bucket_count.data(), _Plb_ptr, std::plus<>{});

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                auto _Value = std::move(_Start[_Idx]);
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                _End[_Plb_ptr[_Byte_idx]++] = _Value;
            }
        }

        // Swap buffer
        std::swap(_Start, _End);
    }
    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v4(std::execution::parallel_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
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

    std::int32_t _Hardware_concurrency = omp_get_num_procs();
    std::size_t _Chunk = (_Size + _Hardware_concurrency - 1) / _Hardware_concurrency;
    std::array<std::size_t, _Bucket_size> _Bucket_count;
    std::vector<std::size_t> _Processor_local_buckets(_Bucket_size * _Hardware_concurrency);

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif

    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::fill(_Plb_ptr, _Plb_ptr + _Bucket_size, 0);

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                ++_Plb_ptr[_Byte_idx];
            }
        }

        std::size_t _Temp_sum{ 0 };
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        for (std::size_t _Idx = 0; _Idx < _Bucket_size; ++_Idx) {
            std::size_t _Temp_sum_local{ 0 };
            for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
                std::size_t _Idx_local = _Bucket_size * _Core + _Idx;
                std::size_t _Temp_count_local = _Processor_local_buckets[_Idx_local];
                _Processor_local_buckets[_Idx_local] = _Temp_sum_local;
                _Temp_sum_local += _Temp_count_local;
            }
            _Bucket_count[_Idx] = _Temp_sum;
            _Temp_sum += _Temp_sum_local;
        }

#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::transform(_Plb_ptr, _Plb_ptr + _Bucket_size, _Bucket_count.data(), _Plb_ptr, std::plus<>{});

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                auto _Value = std::move(_Start[_Idx]);
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                _End[_Plb_ptr[_Byte_idx]++] = _Value;
            }
        }

        // Swap buffer
        std::swap(_Start, _End);
    }
    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}


template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v5(std::execution::parallel_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
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
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
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
                                _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                            else
                                _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
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
                std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread);
                std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread);
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
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
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
                                _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                            else
                                _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

        // 交换缓冲区，准备下一轮
        std::swap(_Start, _End);
    }

    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}


template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v6(std::execution::parallel_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
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

    std::int32_t _Hardware_concurrency = omp_get_num_procs();
    std::size_t _Chunk = (_Size + _Hardware_concurrency - 1) / _Hardware_concurrency;
    std::array<std::size_t, _Bucket_size> _Bucket_count;
    std::vector<std::size_t> _Processor_local_buckets(_Bucket_size * _Hardware_concurrency);

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif

    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::fill(_Plb_ptr, _Plb_ptr + _Bucket_size, 0);

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                ++_Plb_ptr[_Byte_idx];
            }
        }
        /*
        std::array<std::size_t, _Bucket_size> _Bucket_prefix{};

        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = 0; _Idx < _Bucket_size; ++_Idx) {
                std::size_t _Idx_local = _Bucket_size * _Core + _Idx;
                std::size_t _Temp_count_local = _Processor_local_buckets[_Idx_local];

                _Processor_local_buckets[_Idx_local] = _Bucket_prefix[_Idx];
                _Bucket_prefix[_Idx] += _Temp_count_local;
            }
        }

        std::exclusive_scan(_Bucket_prefix.begin(), _Bucket_prefix.end(), _Bucket_count.begin(), 0);
        */
        std::size_t _Temp_sum{ 0 };
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        for (std::size_t _Idx = 0; _Idx < _Bucket_size; ++_Idx) {
            std::size_t _Temp_sum_local{ 0 };
            for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
                std::size_t _Idx_local = _Bucket_size * _Core + _Idx;
                std::size_t _Temp_count_local = _Processor_local_buckets[_Idx_local];
                _Processor_local_buckets[_Idx_local] = _Temp_sum_local;
                _Temp_sum_local += _Temp_count_local;
            }
            _Bucket_count[_Idx] = _Temp_sum;
            _Temp_sum += _Temp_sum_local;
        }

#pragma omp parallel for schedule(static, 1)
        for (std::int32_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            std::transform(_Plb_ptr, _Plb_ptr + _Bucket_size, _Bucket_count.data(), _Plb_ptr, std::plus<>{});

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                auto _Value = std::move(_Start[_Idx]);
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                _End[_Plb_ptr[_Byte_idx]++] = _Value;
            }
        }

        // Swap buffer
        std::swap(_Start, _End);
    }
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

    const auto& test_data = generate_vec_data<T>(size, min_val, max_val);

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

struct ParallelRadixSortV3Wrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        radix_sort_v3<Iter, Compare, Extractor, 256U>(
            std::execution::par, first, last, std::forward<Extractor>(extractor));
    }
};

struct ParallelRadixSortV4Wrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        radix_sort_v4<Iter, Compare, Extractor, 256U>(
            std::execution::par, first, last, std::forward<Extractor>(extractor));
    }
};

struct ParallelRadixSortV5Wrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        radix_sort_v5<Iter, Compare, Extractor, 256U>(
            std::execution::par, first, last, std::forward<Extractor>(extractor));
    }
};

struct ParallelStdSortWrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        // 使用并行版本的 std::sort
        std::sort(std::execution::par, first, last, comp);
    }
};

// 32位类型并行测试
BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int32_t, ParallelRadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int32_t, ParallelRadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int32_t, ParallelRadixSortV3Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int32_t, ParallelRadixSortV4Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int32_t, ParallelRadixSortV5Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int32_t, ParallelStdSortWrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

// 64位类型并行测试
BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int64_t, ParallelRadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int64_t, ParallelRadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int64_t, ParallelRadixSortV3Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int64_t, ParallelRadixSortV4Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int64_t, ParallelRadixSortV5Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_ParallelSort_Typed_Helper, int64_t, ParallelStdSortWrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_MAIN();

/*
2025-11-26T09:12:55+08:00
Running .\Cplusplus-learn.exe
Run on (8 X 2400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 8192 KiB (x1)
----------------------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations UserCounters...
----------------------------------------------------------------------------------
BM_RadixSort_v1/1024         0.043 ms        0.043 ms        16000 bytes_per_second=90.9091Mi/s
BM_RadixSort_v1/2048         0.050 ms        0.052 ms        11200 bytes_per_second=151.351Mi/s
BM_RadixSort_v1/4096         0.071 ms        0.071 ms         8960 bytes_per_second=218.537Mi/s
BM_RadixSort_v1/8192         0.090 ms        0.090 ms         7467 bytes_per_second=347.302Mi/s
BM_RadixSort_v1/16384        0.112 ms        0.098 ms         5600 bytes_per_second=640Mi/s
BM_RadixSort_v1/32768        0.143 ms        0.135 ms         4978 bytes_per_second=926.14Mi/s
BM_RadixSort_v1/65536        0.239 ms        0.234 ms         2800 bytes_per_second=1.04167Gi/s
BM_RadixSort_v1/131072       0.642 ms        0.610 ms          896 bytes_per_second=819.2Mi/s
BM_RadixSort_v1/262144        1.10 ms         1.03 ms          640 bytes_per_second=975.238Mi/s
BM_RadixSort_v1/524288        2.19 ms         2.23 ms          280 bytes_per_second=896Mi/s
BM_RadixSort_v1/1048576       4.90 ms         4.61 ms          149 bytes_per_second=866.909Mi/s
BM_RadixSort_v1_BigO          0.23 NlgN       0.22 NlgN
BM_RadixSort_v1_RMS              8 %             6 %
BM_RadixSort_v2/1024         0.043 ms        0.044 ms        16000 bytes_per_second=88.8889Mi/s
BM_RadixSort_v2/2048         0.051 ms        0.050 ms        10000 bytes_per_second=156.25Mi/s
BM_RadixSort_v2/4096         0.068 ms        0.070 ms        11200 bytes_per_second=224Mi/s
BM_RadixSort_v2/8192         0.087 ms        0.091 ms         8960 bytes_per_second=344.615Mi/s
BM_RadixSort_v2/16384        0.108 ms        0.106 ms         5600 bytes_per_second=589.474Mi/s
BM_RadixSort_v2/32768        0.148 ms        0.141 ms         4978 bytes_per_second=884.978Mi/s
BM_RadixSort_v2/65536        0.253 ms        0.255 ms         3733 bytes_per_second=979.148Mi/s
BM_RadixSort_v2/131072       0.643 ms        0.732 ms          896 bytes_per_second=682.667Mi/s
BM_RadixSort_v2/262144        1.11 ms         1.10 ms          498 bytes_per_second=910.629Mi/s
BM_RadixSort_v2/524288        2.11 ms         2.04 ms          345 bytes_per_second=981.333Mi/s
BM_RadixSort_v2/1048576       4.33 ms         4.12 ms          167 bytes_per_second=971.636Mi/s
BM_RadixSort_v2_BigO          4.13 N          3.95 N
BM_RadixSort_v2_RMS              6 %            10 %
BM_RadixSort_v3/1024         0.038 ms        0.038 ms        18667 bytes_per_second=103.706Mi/s
BM_RadixSort_v3/2048         0.046 ms        0.046 ms        14933 bytes_per_second=169.693Mi/s
BM_RadixSort_v3/4096         0.063 ms        0.064 ms        11200 bytes_per_second=243.478Mi/s
BM_RadixSort_v3/8192         0.083 ms        0.082 ms         7467 bytes_per_second=382.923Mi/s
BM_RadixSort_v3/16384        0.107 ms        0.115 ms         6400 bytes_per_second=544.681Mi/s
BM_RadixSort_v3/32768        0.150 ms        0.137 ms         5600 bytes_per_second=914.286Mi/s
BM_RadixSort_v3/65536        0.227 ms        0.229 ms         3200 bytes_per_second=1.06383Gi/s
BM_RadixSort_v3/131072       0.666 ms        0.698 ms          896 bytes_per_second=716.8Mi/s
BM_RadixSort_v3/262144        1.09 ms         1.05 ms          640 bytes_per_second=952.558Mi/s
BM_RadixSort_v3/524288        2.18 ms         2.30 ms          299 bytes_per_second=869.818Mi/s
BM_RadixSort_v3/1048576       4.20 ms         4.02 ms          179 bytes_per_second=996.174Mi/s
BM_RadixSort_v3_BigO          4.05 N          3.96 N
BM_RadixSort_v3_RMS              7 %            13 %
BM_RadixSort_v4/1024         0.038 ms        0.038 ms        18667 bytes_per_second=103.706Mi/s
BM_RadixSort_v4/2048         0.046 ms        0.046 ms        14452 bytes_per_second=168.047Mi/s
BM_RadixSort_v4/4096         0.062 ms        0.063 ms        11200 bytes_per_second=248.889Mi/s
BM_RadixSort_v4/8192         0.083 ms        0.084 ms         7467 bytes_per_second=373.35Mi/s
BM_RadixSort_v4/16384        0.107 ms        0.110 ms         6400 bytes_per_second=568.889Mi/s
BM_RadixSort_v4/32768        0.144 ms        0.146 ms         4073 bytes_per_second=857.474Mi/s
BM_RadixSort_v4/65536        0.252 ms        0.251 ms         2987 bytes_per_second=995.667Mi/s
BM_RadixSort_v4/131072       0.657 ms        0.703 ms         1000 bytes_per_second=711.111Mi/s
BM_RadixSort_v4/262144        1.13 ms         1.13 ms          498 bytes_per_second=885.333Mi/s
BM_RadixSort_v4/524288        2.19 ms         2.13 ms          448 bytes_per_second=940.066Mi/s
BM_RadixSort_v4/1048576       4.24 ms         4.00 ms          172 bytes_per_second=1000.73Mi/s
BM_RadixSort_v4_BigO          4.09 N          3.90 N
BM_RadixSort_v4_RMS              7 %            10 %
BM_RadixSort_v5/1024         0.011 ms        0.011 ms        64000 bytes_per_second=363.636Mi/s
BM_RadixSort_v5/2048         0.041 ms        0.042 ms        18667 bytes_per_second=186.67Mi/s
BM_RadixSort_v5/4096         0.069 ms        0.072 ms        10000 bytes_per_second=217.391Mi/s
BM_RadixSort_v5/8192         0.086 ms        0.088 ms        11200 bytes_per_second=355.556Mi/s
BM_RadixSort_v5/16384        0.112 ms        0.106 ms         5600 bytes_per_second=589.474Mi/s
BM_RadixSort_v5/32768        0.135 ms        0.128 ms         5600 bytes_per_second=973.913Mi/s
BM_RadixSort_v5/65536        0.226 ms        0.234 ms         3200 bytes_per_second=1.04167Gi/s
BM_RadixSort_v5/131072       0.629 ms        0.670 ms         1120 bytes_per_second=746.667Mi/s
BM_RadixSort_v5/262144       0.993 ms        0.994 ms          896 bytes_per_second=1006.04Mi/s
BM_RadixSort_v5/524288        1.90 ms         1.88 ms          407 bytes_per_second=1.03827Gi/s
BM_RadixSort_v5/1048576       3.83 ms         3.93 ms          187 bytes_per_second=1018.55Mi/s
BM_RadixSort_v5_BigO          3.67 N          3.73 N
BM_RadixSort_v5_RMS              8 %             9 %

第一名：v5 - 在大部分数据规模下表现最佳

第二名：v3 - 中等规模数据表现稳定

第三名：v4 - 与v3相当，略逊一筹

第四/五名：v1 v2 相当 基本无差异
*/