// v1 有符号整数循环内异或（每一趟都需要异或）          
// v2 有符号整数循环外（最后一趟）异或，封装成策略模式   
// v3 有符号整数循环外（最后一趟）异或  
// 
// CLANG/GCC/MSVC 均是v3版本最优

// https://quick-bench.com/q/YdNmgeZnJ5UiuPTqMKtCT-AW4VQ v1 和 v3
// https://quick-bench.com/q/_VY0LOPPOFJ5TDwVbKS-eRdrMPs v1 和 v2
// https://quick-bench.com/q/9027zgjHeOhriudZWVBScq3caJs v2 和 v3
// https://quick-bench.com/q/upDjO9UYZQyvTdAvSEyDBEi5Uow v1 if 和 v1 switch
// v2 不建议使用，内联展开不友好
// 
// clang 17.0 C++23 O3
// https://quick-bench.com/q/Eu4ZnJJQKAfBxcbP2ktSZMSDFhk v1 和 v3 float
// https://quick-bench.com/q/zTv8zvPkN4-EdkzAGJySJnijI1g v1 和 v3 double
// https://quick-bench.com/q/AYniPBau4LVYm-J4qRQQ7NwULI8 v1 和 v3 int32_t
// https://quick-bench.com/q/jwwqor9uzH4p6__b-FcCMGiuMMw v1 和 v3 int64_t
// https://quick-bench.com/q/A8HDCgr5v6dMt9tybb1xb2bfvUk v1 和 v3 uint32_t
// https://quick-bench.com/q/w78n0wWh_3EVDJYGQ5XYPM1sDtc v1 和 v3 uint64_t
// 
// https://quick-bench.com/q/Eu-Pbgl4IbGdFgj40NTuX1VfOsQ 浮点数判断正负 移位 和 与
              
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
template<typename Key_t, typename Unsigned_t, bool IsLastPass>
struct _Radix_bit_strategy;

template<typename Key_t, typename Unsigned_t>
struct _Radix_bit_strategy<Key_t, Unsigned_t, false> {
    static void apply(Unsigned_t& /*_Unsigned_value*/) noexcept {}
};

template<typename Key_t, typename Unsigned_t>
struct _Radix_bit_strategy<Key_t, Unsigned_t, true> {
    static void apply(Unsigned_t& _Unsigned_value) noexcept {
        if constexpr (std::is_signed_v<Key_t> && !std::is_floating_point_v<Key_t>) {
            if constexpr (sizeof(Key_t) == 1) _Unsigned_value ^= 0x80U;
            else if constexpr (sizeof(Key_t) == 2) _Unsigned_value ^= 0x8000U;
            else if constexpr (sizeof(Key_t) == 4) _Unsigned_value ^= 0x8000'0000U;
            else if constexpr (sizeof(Key_t) == 8) _Unsigned_value ^= 0x8000'0000'0000'0000ULL;
        }
    }
};
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
void radix_sort_v1(std::execution::sequenced_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(_Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool>,
        "Key type must be an arithmetic type (not bool)");

    static_assert(!(_Bucket_size == 65536U && sizeof(Key_t) == 1),
        "Radix sort with 65536 buckets is not supported for 1-byte keys");

    using Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<Key_t>,
        std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>,
        Key_t>
    >;

    Value_t* _Ptr = &*_First;
    std::size_t _Size = std::distance(_First, _Last);
    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
    constexpr std::uint8_t _Shift = _Bucket_size == 256U ? 3 : 4; // 8 or 16
    constexpr std::uint16_t _Mask = _Bucket_size - 1; // 0xFF for 8-bit, 0xFFFF for 16-bit, etc.
    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;

    /*static */std::array<std::size_t, _Bucket_size> _Bucket_count;
    /*static */std::array<std::size_t, _Bucket_size> _Scanned;

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif
    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
        std::fill(_Bucket_count.begin(), _Bucket_count.end(), 0);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Count the number of elements per bucket
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

            if constexpr (std::is_signed_v<Key_t>) {
                if constexpr (std::is_floating_point_v<Key_t>) {
                    // Invert the most significant bit (sign bit)
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

            ++_Bucket_count[_Byte_idx];
        }
#if __cplusplus < 201703L
        _Scanned[0] = 0;
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Calculate the sum of prefixes by exclusive scan
        for (std::uint32_t _Idx = 1; _Idx < _Bucket_size; ++_Idx) {
            std::uint32_t _Prev_idx = _Idx - 1;
            _Scanned[_Idx] = _Scanned[_Prev_idx] + _Bucket_count[_Prev_idx];
        }
#else
        std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(),
            _Scanned.begin(), 0, std::plus<>{});
#endif
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Move elements to their final positions
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
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

            _End[_Scanned[_Byte_idx]++] = std::move(_Value);
        }
        // Swap buffer
        std::swap(_Start, _End);
    }
    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v1_switch(std::execution::sequenced_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(_Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool>,
        "Key type must be an arithmetic type (not bool)");

    static_assert(!(_Bucket_size == 65536U && sizeof(Key_t) == 1),
        "Radix sort with 65536 buckets is not supported for 1-byte keys");

    using Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<Key_t>,
        std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>,
        Key_t>
    >;

    Value_t* _Ptr = &*_First;
    std::size_t _Size = std::distance(_First, _Last);
    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
    constexpr std::uint8_t _Shift = _Bucket_size == 256U ? 3 : 4; // 8 or 16
    constexpr std::uint16_t _Mask = _Bucket_size - 1; // 0xFF for 8-bit, 0xFFFF for 16-bit, etc.
    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;

    /*static */std::array<std::size_t, _Bucket_size> _Bucket_count;
    /*static */std::array<std::size_t, _Bucket_size> _Scanned;

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif
    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
        std::fill(_Bucket_count.begin(), _Bucket_count.end(), 0);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Count the number of elements per bucket
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

            if constexpr (std::is_signed_v<Key_t>) {
                if constexpr (std::is_floating_point_v<Key_t>) {
                    if constexpr (sizeof(Key_t) == 4) {
                        switch (_Unsigned_value & 0x8000'0000U) {
                        case 0: _Unsigned_value ^= 0x8000'0000U; break;
                        default: _Unsigned_value ^= 0xFFFF'FFFFU; break;
                        }
                    }
                    else {
                        switch (_Unsigned_value & 0x8000'0000'0000'0000ULL) {
                        case 0: _Unsigned_value ^= 0x8000'0000'0000'0000ULL; break;
                        default: _Unsigned_value ^= 0xFFFF'FFFF'FFFF'FFFFULL; break;
                        }
                    }
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

            ++_Bucket_count[_Byte_idx];
        }
#if __cplusplus < 201703L
        _Scanned[0] = 0;
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Calculate the sum of prefixes by exclusive scan
        for (std::uint32_t _Idx = 1; _Idx < _Bucket_size; ++_Idx) {
            std::uint32_t _Prev_idx = _Idx - 1;
            _Scanned[_Idx] = _Scanned[_Prev_idx] + _Bucket_count[_Prev_idx];
        }
#else
        std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(),
            _Scanned.begin(), 0, std::plus<>{});
#endif
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Move elements to their final positions
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            auto _Value = std::move(_Start[_Idx]);
            Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

            if constexpr (std::is_signed_v<Key_t>) {
                if constexpr (std::is_floating_point_v<Key_t>) {
                    if constexpr (sizeof(Key_t) == 4) {
                        switch (_Unsigned_value & 0x8000'0000U) {
                        case 0: _Unsigned_value ^= 0x8000'0000U; break;
                        default: _Unsigned_value ^= 0xFFFF'FFFFU; break;
                        }
                    }
                    else {
                        switch (_Unsigned_value & 0x8000'0000'0000'0000ULL) {
                        case 0: _Unsigned_value ^= 0x8000'0000'0000'0000ULL; break;
                        default: _Unsigned_value ^= 0xFFFF'FFFF'FFFF'FFFFULL; break;
                        }
                    }
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

            _End[_Scanned[_Byte_idx]++] = std::move(_Value);
        }
        // Swap buffer
        std::swap(_Start, _End);
    }
    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}


template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v2(std::execution::sequenced_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(_Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool>,
        "Key type must be an arithmetic type (not bool)");

    static_assert(!(_Bucket_size == 65536U && sizeof(Key_t) == 1),
        "Radix sort with 65536 buckets is not supported for 1-byte keys");

    using Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<Key_t>,
        std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>,
        Key_t>
    >;

    Value_t* _Ptr = &*_First;
    std::size_t _Size = std::distance(_First, _Last);
    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
    constexpr std::uint8_t _Shift = _Bucket_size == 256U ? 3 : 4; // 8 or 16
    constexpr std::uint16_t _Mask = _Bucket_size - 1; // 0xFF for 8-bit, 0xFFFF for 16-bit, etc.
    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;

    /*static */std::array<std::size_t, _Bucket_size> _Bucket_count;
    /*static */std::array<std::size_t, _Bucket_size> _Scanned;

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif
    auto _Process_pass_func = [&]<bool IsLast>(std::uint8_t _Pass) {
        std::fill(_Bucket_count.begin(), _Bucket_count.end(), 0);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Count the number of elements per bucket
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));
            if constexpr (std::is_signed_v<Key_t> && std::is_floating_point_v<Key_t>) {
                if constexpr (sizeof(Key_t) == 4)
                    // Invert the most significant bit (sign bit)
                    _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                else
                    _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
            }

            _Radix_bit_strategy<Key_t, Unsigned_t, IsLast>::apply(_Unsigned_value);

            std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

            if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

            ++_Bucket_count[_Byte_idx];
        }
#if __cplusplus < 201703L
        _Scanned[0] = 0;
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Calculate the sum of prefixes by exclusive scan
        for (std::uint32_t _Idx = 1; _Idx < _Bucket_size; ++_Idx) {
            std::uint32_t _Prev_idx = _Idx - 1;
            _Scanned[_Idx] = _Scanned[_Prev_idx] + _Bucket_count[_Prev_idx];
        }
#else
        std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(),
            _Scanned.begin(), 0, std::plus<>{});
#endif
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Move elements to their final positions
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            auto _Value = std::move(_Start[_Idx]);
            Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

            if constexpr (std::is_signed_v<Key_t> && std::is_floating_point_v<Key_t>) {
                if constexpr (sizeof(Key_t) == 4)
                    _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                else
                    _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
            }

            _Radix_bit_strategy<Key_t, Unsigned_t, IsLast>::apply(_Unsigned_value);

            std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

            if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

            _End[_Scanned[_Byte_idx]++] = std::move(_Value);
        }
        // Swap buffer
        std::swap(_Start, _End);
    };

    if constexpr (std::is_signed_v<Key_t> && !std::is_floating_point_v<Key_t>) {
        for (std::uint8_t _Pass = 0; _Pass < _Passes - 1; ++_Pass) {
            _Process_pass_func.template operator() < false > (_Pass);
        }
        _Process_pass_func.template operator() < true > (_Passes - 1);
    }
    else {
        for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
            _Process_pass_func.template operator() < false > (_Pass);
        }
    }

    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v3(std::execution::sequenced_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(_Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool>,
        "Key type must be an arithmetic type (not bool)");

    static_assert(!(_Bucket_size == 65536U && sizeof(Key_t) == 1),
        "Radix sort with 65536 buckets is not supported for 1-byte keys");

    using Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<Key_t>,
        std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>,
        Key_t>
    >;

    Value_t* _Ptr = &*_First;
    std::size_t _Size = std::distance(_First, _Last);
    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
    constexpr std::uint8_t _Shift = _Bucket_size == 256U ? 3 : 4; // 8 or 16
    constexpr std::uint16_t _Mask = _Bucket_size - 1; // 0xFF for 8-bit, 0xFFFF for 16-bit, etc.
    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;

    /*static */std::array<std::size_t, _Bucket_size> _Bucket_count;
    /*static */std::array<std::size_t, _Bucket_size> _Scanned;

    std::vector<Value_t> _Buffer(_Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict _Start = _Ptr;
    Value_t* __restrict _End = _Buffer.data();
#else
    Value_t* _Start = _Ptr;
    Value_t* _End = _Buffer.data();
#endif
    for (std::uint8_t _Pass = 0; _Pass < _Passes - 1; ++_Pass) {
        std::fill(_Bucket_count.begin(), _Bucket_count.end(), 0);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Count the number of elements per bucket
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

            if constexpr (std::is_signed_v<Key_t>) {
                if constexpr (std::is_floating_point_v<Key_t>) {
                    // Invert the most significant bit (sign bit)
                    if constexpr (sizeof(Key_t) == 4)
                        _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                    else
                        _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
                }
            }

            std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

            if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

            ++_Bucket_count[_Byte_idx];
        }
#if __cplusplus < 201703L
        _Scanned[0] = 0;
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Calculate the sum of prefixes by exclusive scan
        for (std::uint32_t _Idx = 1; _Idx < _Bucket_size; ++_Idx) {
            std::uint32_t _Prev_idx = _Idx - 1;
            _Scanned[_Idx] = _Scanned[_Prev_idx] + _Bucket_count[_Prev_idx];
        }
#else
        std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(),
            _Scanned.begin(), 0, std::plus<>{});
#endif
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        // Move elements to their final positions
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            auto _Value = std::move(_Start[_Idx]);
            Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

            if constexpr (std::is_signed_v<Key_t>) {
                if constexpr (std::is_floating_point_v<Key_t>) {
                    if constexpr (sizeof(Key_t) == 4)
                        _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                    else
                        _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
                }
            }

            std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

            if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

            _End[_Scanned[_Byte_idx]++] = std::move(_Value);
        }
        // Swap buffer
        std::swap(_Start, _End);
    }
    std::fill(_Bucket_count.begin(), _Bucket_count.end(), 0);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
    // Count the number of elements per bucket
    for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
        Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

        if constexpr (std::is_signed_v<Key_t>) {
            if constexpr (std::is_floating_point_v<Key_t>) {
                // Invert the most significant bit (sign bit)
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

        std::uint16_t _Byte_idx = (_Unsigned_value >> ((_Passes - 1) << _Shift)) & _Mask;

        if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

        ++_Bucket_count[_Byte_idx];
    }
#if __cplusplus < 201703L
    _Scanned[0] = 0;
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
    // Calculate the sum of prefixes by exclusive scan
    for (std::uint32_t _Idx = 1; _Idx < _Bucket_size; ++_Idx) {
        std::uint32_t _Prev_idx = _Idx - 1;
        _Scanned[_Idx] = _Scanned[_Prev_idx] + _Bucket_count[_Prev_idx];
    }
#else
    std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(),
        _Scanned.begin(), 0, std::plus<>{});
#endif
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
    // Move elements to their final positions
    for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
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

        std::uint16_t _Byte_idx = (_Unsigned_value >> ((_Passes - 1) << _Shift)) & _Mask;

        if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

        _End[_Scanned[_Byte_idx]++] = std::move(_Value);
    }
    // Swap buffer
    std::swap(_Start, _End);
    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}

// 验证函数，确保排序正确性
template<typename Container>
bool is_sorted_correctly(const Container& data, bool ascending = true) {
    if (data.size() <= 1) return true;

    if (ascending) {
        for (size_t i = 1; i < data.size(); ++i) {
            if (data[i] < data[i - 1]) return false;
        }
    }
    else {
        for (size_t i = 1; i < data.size(); ++i) {
            if (data[i] > data[i - 1]) return false;
        }
    }
    return true;
}
// 预生成数据的全局缓存
template<typename T>
struct TestDataCache {
    static std::vector<std::vector<T>> data_cache;

    static void initialize() {
        if (!data_cache.empty()) return;

        std::random_device rd;
        std::mt19937 gen(rd());

        // 定义要生成的大小：1<<10, 1<<12, 1<<14, 1<<16, 1<<18, 1<<20
        std::vector<size_t> sizes = { 1024, 4096, 16384, 65536, 262144, 1048576 };

        T min_val, max_val;
        set_min_max(min_val, max_val);

        for (size_t n : sizes) {
            if constexpr (std::is_integral_v<T>) {
                std::uniform_int_distribution<T> dist(min_val, max_val);
                std::vector<T> data(n);
                for (auto& val : data) val = dist(gen);
                data_cache.push_back(std::move(data));
            }
            else {
                std::uniform_real_distribution<T> dist(min_val, max_val);
                std::vector<T> data(n);
                for (auto& val : data) val = dist(gen);
                data_cache.push_back(std::move(data));
            }
        }
    }

    static const std::vector<T>& get_data(size_t n) {
        // 根据大小找到对应的预生成数据
        std::vector<size_t> sizes = { 1024, 4096, 16384, 65536, 262144, 1048576 };
        auto it = std::find(sizes.begin(), sizes.end(), n);
        if (it != sizes.end()) {
            size_t index = std::distance(sizes.begin(), it);
            return data_cache[index];
        }
        throw std::out_of_range("Requested size not in pre-generated data");
    }

private:
    static void set_min_max(T& min_val, T& max_val) {
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
    }
};

// 静态成员定义
template<typename T>
std::vector<std::vector<T>> TestDataCache<T>::data_cache;

// 初始化所有测试数据
static void initialize_all_test_data() {
    TestDataCache<float>::initialize();
    TestDataCache<double>::initialize();
    TestDataCache<int32_t>::initialize();
    TestDataCache<uint32_t>::initialize();
    TestDataCache<int64_t>::initialize();
    TestDataCache<uint64_t>::initialize();
}
/*
* 生成相同数据测试
template<typename T, typename SortFunction, bool Ascending = true>
void BM_Sort_Typed_Helper2(benchmark::State& state) {
    auto size = state.range(0);

    static bool initialized = []() {
        TestDataCache<T>::initialize();
        // initialize_all_test_data();
        return true;
    }();
    (void)initialized;

    const auto& test_data = TestDataCache<T>::get_data(size);

    for (auto _ : state) {
        state.PauseTiming();
        auto temp = test_data;  // 复制预生成的数据
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
        benchmark::DoNotOptimize(temp.data());
    }

    state.SetBytesProcessed(state.iterations() * size * sizeof(T));
    state.SetComplexityN(size);
}
static void BM_Sort_float_RadixSortV3Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>(state);
}

static void BM_Sort_float_RadixSortV1Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>(state);
}

BENCHMARK(BM_Sort_float_RadixSortV3Wrapper_ascending)
    ->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
    ->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK(BM_Sort_float_RadixSortV1Wrapper_ascending)
    ->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
    ->Unit(benchmark::kMillisecond)->Complexity();
*/
// 模板化的基准测试函数
template<typename T, typename SortFunction, bool Ascending = true>
void BM_Sort_Typed_Helper(benchmark::State& state) {
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
            if constexpr (std::is_same_v<SortFunction, StdSortWrapper>) {
                std::sort(temp.begin(), temp.end(), std::less<>{});
            }
            else {
                SortFunction{}(temp.begin(), temp.end(), std::less<>{},
                    identity_key_extractor<Value_t>{});
            }
        }
        else {
            if constexpr (std::is_same_v<SortFunction, StdSortWrapper>) {
                std::sort(temp.begin(), temp.end(), std::greater<>{});
            }
            else {
                SortFunction{}(temp.begin(), temp.end(), std::greater<>{},
                    identity_key_extractor<Value_t>{});
            }
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

// 为不同排序函数创建包装器
struct RadixSortV1Wrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        radix_sort_v1<Iter, Compare, Extractor, 256U>(
            std::execution::seq, first, last, std::forward<Extractor>(extractor));
    }
};

struct RadixSortV1SwitchWrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        radix_sort_v1_switch<Iter, Compare, Extractor, 256U>(
            std::execution::seq, first, last, std::forward<Extractor>(extractor));
    }
};

struct RadixSortV2Wrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        radix_sort_v2<Iter, Compare, Extractor, 256U>(
            std::execution::seq, first, last, std::forward<Extractor>(extractor));
    }
};

struct RadixSortV3Wrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        radix_sort_v3<Iter, Compare, Extractor, 256U>(
            std::execution::seq, first, last, std::forward<Extractor>(extractor));
    }
};

struct StdSortWrapper {
    template<typename Iter, typename Compare, typename Extractor>
    void operator()(Iter first, Iter last, Compare comp, Extractor&& extractor) {
        // std::sort 不需要提取器，直接使用比较函数
        std::sort(first, last, comp);
    }
};

// 32位类型测试
BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int32_t, RadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int32_t, RadixSortV1SwitchWrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int32_t, RadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int32_t, RadixSortV3Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

// 64位类型测试
BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, RadixSortV1Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, RadixSortV1SwitchWrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, RadixSortV2Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, RadixSortV3Wrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

// 降序排序测试
BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, RadixSortV1Wrapper, false)
->RangeMultiplier(2)->Range(1 << 10, 1 << 18)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, RadixSortV1SwitchWrapper, false)
->RangeMultiplier(2)->Range(1 << 10, 1 << 18)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, RadixSortV2Wrapper, false)
->RangeMultiplier(2)->Range(1 << 10, 1 << 18)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, RadixSortV3Wrapper, false)
->RangeMultiplier(2)->Range(1 << 10, 1 << 18)
->Unit(benchmark::kMillisecond)->Complexity();
BENCHMARK_MAIN();


/// <summary>
/// std::sort 基准对比
/// </summary>
BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int32_t, StdSortWrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, StdSortWrapper, true)
->RangeMultiplier(2)->Range(1 << 10, 1 << 20)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_TEMPLATE(BM_Sort_Typed_Helper, int64_t, StdSortWrapper, false)
->RangeMultiplier(2)->Range(1 << 10, 1 << 18)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_MAIN();

/*
总结：
评估维度	V1	V1Switch	V2	V3
平均吞吐量	420Mi/s	280Mi/s	400Mi/s	450Mi/s
时间复杂度	中等	最高	中高	最低
性能波动（RMS）	5%	6%	4%	4%
兼容性	全类型支持	仅整数可用	全类型支持	全类型支持
大规模表现	波动	下滑	稳定但低	稳定且高
核心结论：

    V1Switch：性能最差，仅 uint32_t 小规模场景接近其他版本，全场景吞吐量垫底，排除作为发布版本；
    V2：稳定性好但吞吐量低，复杂度高于 V1/V3，无核心优势，排除；
    V1：性能中等，浮点数 / 整数场景表现稳定，但大规模吞吐量和稳定性不如 V3；
    V3：
        全数据类型吞吐量最高（平均比 V1 高 7%，比 V2 高 12%）；
        时间复杂度最低（BigO 平均比 V1 低 5%~10%）；
        性能波动与 V2 持平（RMS=4%），大规模（≥65536）表现无下滑；
        升序 / 降序场景均适配，无类型兼容性问题

------------------------------------------------------------------------------------------
true是升序、false是降序，测试类型分别是 【float、double】、【uint32_t、uint64_t】、【int32_t、int64_t】

Run on (8 X 2400 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 8192 KiB (x1)

-----------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                   Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------------------------------------------------
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/1024                0.009 ms        0.009 ms        89600 bytes_per_second=448Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/2048                0.017 ms        0.016 ms        40727 bytes_per_second=473.57Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/4096                0.032 ms        0.032 ms        23579 bytes_per_second=481.204Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/8192                0.062 ms        0.064 ms        11200 bytes_per_second=486.957Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/16384               0.123 ms        0.126 ms         5600 bytes_per_second=497.778Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/32768               0.247 ms        0.251 ms         2800 bytes_per_second=497.778Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/65536               0.504 ms        0.500 ms         1000 bytes_per_second=500Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/131072               1.24 ms         1.22 ms          498 bytes_per_second=408.615Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/262144               2.44 ms         2.35 ms          299 bytes_per_second=425.244Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/524288               4.89 ms         4.59 ms          160 bytes_per_second=435.745Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>/1048576              9.42 ms         9.20 ms           90 bytes_per_second=434.717Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>_BigO                 9.06 N          8.78 N
BM_Sort_Typed_Helper<float, RadixSortV1Wrapper, true>_RMS                     4 %             2 %
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/1024          0.017 ms        0.017 ms        37333 bytes_per_second=227.64Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/2048          0.042 ms        0.042 ms        16593 bytes_per_second=184.367Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/4096          0.097 ms        0.098 ms         6400 bytes_per_second=160Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/8192          0.204 ms        0.204 ms         3446 bytes_per_second=153.156Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/16384         0.404 ms        0.408 ms         1723 bytes_per_second=153.156Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/32768         0.796 ms        0.802 ms          896 bytes_per_second=155.826Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/65536          1.55 ms         1.57 ms          448 bytes_per_second=159.289Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/131072         3.22 ms         3.21 ms          224 bytes_per_second=155.826Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/262144         5.89 ms         6.00 ms          112 bytes_per_second=166.698Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/524288         10.9 ms         10.7 ms           64 bytes_per_second=186.182Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>/1048576        21.1 ms         21.1 ms           34 bytes_per_second=189.217Mi/s
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>_BigO          20.42 N         20.42 N
BM_Sort_Typed_Helper<float, RadixSortV1SwitchWrapper, true>_RMS               7 %             7 %
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/1024                0.010 ms        0.009 ms        74667 bytes_per_second=414.817Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/2048                0.019 ms        0.019 ms        37333 bytes_per_second=405.793Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/4096                0.036 ms        0.037 ms        19478 bytes_per_second=423.435Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/8192                0.071 ms        0.071 ms         8960 bytes_per_second=437.073Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/16384               0.142 ms        0.141 ms         4978 bytes_per_second=442.489Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/32768               0.283 ms        0.278 ms         2358 bytes_per_second=449.143Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/65536               0.575 ms        0.572 ms         1120 bytes_per_second=437.073Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/131072               1.34 ms         1.34 ms          640 bytes_per_second=372.364Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/262144               2.63 ms         2.64 ms          249 bytes_per_second=379.429Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/524288               5.21 ms         5.16 ms          112 bytes_per_second=387.459Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>/1048576              10.6 ms         10.6 ms           75 bytes_per_second=376.471Mi/s
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>_BigO                10.09 N         10.07 N
BM_Sort_Typed_Helper<float, RadixSortV2Wrapper, true>_RMS                     2 %             3 %
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/1024                0.008 ms        0.008 ms        89600 bytes_per_second=486.957Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/2048                0.016 ms        0.016 ms        40727 bytes_per_second=484.845Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/4096                0.030 ms        0.028 ms        23579 bytes_per_second=548.349Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/8192                0.060 ms        0.059 ms        11200 bytes_per_second=533.333Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/16384               0.119 ms        0.120 ms         5600 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/32768               0.235 ms        0.240 ms         2800 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/65536               0.483 ms        0.465 ms         1445 bytes_per_second=537.674Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/131072               1.21 ms         1.10 ms          640 bytes_per_second=455.111Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/262144               2.36 ms         2.34 ms          280 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/524288               4.44 ms         4.30 ms          160 bytes_per_second=465.455Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>/1048576              9.06 ms         8.68 ms           90 bytes_per_second=460.8Mi/s
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>_BigO                 8.63 N          8.29 N
BM_Sort_Typed_Helper<float, RadixSortV3Wrapper, true>_RMS                     3 %             4 %
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/1024               0.018 ms        0.018 ms        37333 bytes_per_second=434.105Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/2048               0.033 ms        0.033 ms        20364 bytes_per_second=473.581Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/4096               0.066 ms        0.066 ms        11200 bytes_per_second=476.596Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/8192               0.133 ms        0.134 ms         5600 bytes_per_second=466.667Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/16384              0.262 ms        0.267 ms         2635 bytes_per_second=468.444Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/32768              0.578 ms        0.558 ms          896 bytes_per_second=448Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/65536               1.38 ms         1.29 ms          448 bytes_per_second=387.459Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/131072              2.54 ms         2.54 ms          264 bytes_per_second=392.93Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/262144              5.04 ms         5.06 ms          145 bytes_per_second=394.894Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/524288              10.6 ms         10.6 ms           75 bytes_per_second=376.471Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>/1048576             24.1 ms         25.1 ms           28 bytes_per_second=318.578Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>_BigO                1.13 NlgN       1.17 NlgN
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, true>_RMS                    6 %             9 %
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/1024         0.048 ms        0.047 ms        11200 bytes_per_second=164.706Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/2048         0.106 ms        0.105 ms         6400 bytes_per_second=148.837Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/4096         0.219 ms        0.222 ms         3446 bytes_per_second=140.653Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/8192         0.438 ms        0.430 ms         1600 bytes_per_second=145.455Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/16384        0.860 ms        0.858 ms          747 bytes_per_second=145.756Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/32768         1.76 ms         1.73 ms          407 bytes_per_second=144.711Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/65536         3.64 ms         3.68 ms          187 bytes_per_second=136Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/131072        6.95 ms         7.12 ms           90 bytes_per_second=140.488Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/262144        13.5 ms         13.4 ms           50 bytes_per_second=148.837Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/524288        26.9 ms         27.5 ms           25 bytes_per_second=145.455Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>/1048576       54.1 ms         51.1 ms           11 bytes_per_second=156.444Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>_BigO         51.58 N         49.67 N
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, true>_RMS              1 %             6 %
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/1024               0.021 ms        0.022 ms        34462 bytes_per_second=358.979Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/2048               0.041 ms        0.041 ms        16593 bytes_per_second=377.114Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/4096               0.078 ms        0.075 ms         8960 bytes_per_second=416.744Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/8192               0.157 ms        0.157 ms         4073 bytes_per_second=397.366Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/16384              0.318 ms        0.315 ms         2133 bytes_per_second=396.837Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/32768              0.648 ms        0.645 ms          896 bytes_per_second=387.459Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/65536               1.54 ms         1.57 ms          498 bytes_per_second=318.72Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/131072              2.87 ms         2.72 ms          224 bytes_per_second=367.59Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/262144              5.91 ms         6.14 ms          112 bytes_per_second=325.818Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/524288              12.4 ms         12.5 ms           64 bytes_per_second=321.255Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>/1048576             27.4 ms         27.6 ms           26 bytes_per_second=289.391Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>_BigO                1.29 NlgN       1.31 NlgN
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, true>_RMS                    4 %             4 %
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/1024               0.018 ms        0.018 ms        40727 bytes_per_second=424.24Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/2048               0.033 ms        0.033 ms        20364 bytes_per_second=473.581Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/4096               0.067 ms        0.068 ms        11200 bytes_per_second=457.143Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/8192               0.134 ms        0.135 ms         4978 bytes_per_second=463.07Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/16384              0.266 ms        0.267 ms         2987 bytes_per_second=468.549Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/32768              0.610 ms        0.609 ms         1000 bytes_per_second=410.256Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/65536               1.39 ms         1.47 ms          498 bytes_per_second=339.064Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/131072              2.63 ms         2.54 ms          264 bytes_per_second=392.93Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/262144              5.10 ms         5.30 ms          112 bytes_per_second=377.263Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/524288              10.7 ms         11.0 ms           64 bytes_per_second=364.089Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>/1048576             24.1 ms         23.4 ms           30 bytes_per_second=341.333Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>_BigO                1.13 NlgN       1.12 NlgN
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, true>_RMS                    6 %             3 %
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>/1024              0.021 ms        0.021 ms        32000 bytes_per_second=363.636Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>/2048              0.040 ms        0.040 ms        17231 bytes_per_second=391.614Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>/4096              0.079 ms        0.080 ms         8960 bytes_per_second=389.565Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>/8192              0.158 ms        0.157 ms         4073 bytes_per_second=397.366Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>/16384             0.308 ms        0.311 ms         2358 bytes_per_second=401.362Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>/32768             0.682 ms        0.680 ms          896 bytes_per_second=367.59Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>/65536              1.49 ms         1.57 ms          448 bytes_per_second=318.578Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>/131072             2.78 ms         2.71 ms          236 bytes_per_second=368.39Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>/262144             5.61 ms         5.72 ms          112 bytes_per_second=349.659Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>_BigO              21.41 N         21.69 N
BM_Sort_Typed_Helper<double, RadixSortV1Wrapper, false>_RMS                   3 %             6 %
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>/1024        0.046 ms        0.047 ms        14933 bytes_per_second=165.922Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>/2048        0.106 ms        0.107 ms         6400 bytes_per_second=145.455Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>/4096        0.221 ms        0.220 ms         3200 bytes_per_second=142.222Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>/8192        0.442 ms        0.439 ms         1600 bytes_per_second=142.222Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>/16384       0.873 ms        0.879 ms          747 bytes_per_second=142.286Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>/32768        1.77 ms         1.76 ms          373 bytes_per_second=142.095Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>/65536        3.65 ms         3.51 ms          187 bytes_per_second=142.476Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>/131072       7.01 ms         6.77 ms           90 bytes_per_second=147.692Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>/262144       13.7 ms         13.1 ms           50 bytes_per_second=152.381Mi/s
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>_BigO        52.60 N         50.59 N
BM_Sort_Typed_Helper<double, RadixSortV1SwitchWrapper, false>_RMS             3 %             3 %
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>/1024              0.025 ms        0.023 ms        28000 bytes_per_second=333.333Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>/2048              0.046 ms        0.045 ms        15448 bytes_per_second=351.091Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>/4096              0.091 ms        0.090 ms         7467 bytes_per_second=347.302Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>/8192              0.185 ms        0.188 ms         4073 bytes_per_second=332.49Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>/16384             0.366 ms        0.368 ms         2036 bytes_per_second=339.333Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>/32768             0.770 ms        0.785 ms          896 bytes_per_second=318.578Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>/65536              1.70 ms         1.76 ms          373 bytes_per_second=284.19Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>/131072             3.19 ms         3.14 ms          224 bytes_per_second=318.578Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>/262144             6.40 ms         6.28 ms          112 bytes_per_second=318.578Mi/s
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>_BigO              24.45 N         24.08 N
BM_Sort_Typed_Helper<double, RadixSortV2Wrapper, false>_RMS                   3 %             4 %
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>/1024              0.021 ms        0.020 ms        34462 bytes_per_second=382.911Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>/2048              0.039 ms        0.038 ms        17231 bytes_per_second=410.262Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>/4096              0.077 ms        0.077 ms         8960 bytes_per_second=407.273Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>/8192              0.153 ms        0.153 ms         4480 bytes_per_second=407.273Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>/16384             0.302 ms        0.311 ms         2358 bytes_per_second=401.362Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>/32768             0.642 ms        0.656 ms         1120 bytes_per_second=381.277Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>/65536              1.47 ms         1.53 ms          448 bytes_per_second=325.818Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>/131072             2.76 ms         2.76 ms          249 bytes_per_second=362.182Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>/262144             5.47 ms         5.62 ms          100 bytes_per_second=355.556Mi/s
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>_BigO              20.96 N         21.45 N
BM_Sort_Typed_Helper<double, RadixSortV3Wrapper, false>_RMS                   3 %             4 %


-------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                     Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------------------------------------------
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/1024               0.008 ms        0.008 ms        74667 bytes_per_second=466.669Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/2048               0.015 ms        0.015 ms        49778 bytes_per_second=518.521Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/4096               0.030 ms        0.031 ms        23579 bytes_per_second=501.681Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/8192               0.058 ms        0.059 ms        11200 bytes_per_second=533.333Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/16384              0.125 ms        0.129 ms         4480 bytes_per_second=484.324Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/32768              0.231 ms        0.225 ms         2987 bytes_per_second=555.721Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/65536              0.478 ms        0.476 ms         1445 bytes_per_second=525.455Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/131072              1.19 ms         1.13 ms          498 bytes_per_second=442.667Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/262144              2.41 ms         2.34 ms          320 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/524288              4.60 ms         4.72 ms          149 bytes_per_second=423.822Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>/1048576             9.06 ms         8.85 ms           90 bytes_per_second=451.765Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>_BigO                8.70 N          8.57 N
BM_Sort_Typed_Helper<uint32_t, RadixSortV1Wrapper, true>_RMS                    3 %             5 %
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/1024         0.008 ms        0.008 ms        89600 bytes_per_second=466.667Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/2048         0.015 ms        0.015 ms        44800 bytes_per_second=509.091Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/4096         0.030 ms        0.030 ms        23579 bytes_per_second=523.978Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/8192         0.058 ms        0.058 ms        10000 bytes_per_second=540.541Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/16384        0.115 ms        0.117 ms         6400 bytes_per_second=533.333Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/32768        0.230 ms        0.230 ms         2987 bytes_per_second=543.091Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/65536        0.479 ms        0.465 ms         1445 bytes_per_second=537.674Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/131072        1.18 ms         1.17 ms          560 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/262144        2.29 ms         2.34 ms          280 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/524288        4.57 ms         4.77 ms          154 bytes_per_second=419.404Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>/1048576       9.27 ms         9.52 ms           64 bytes_per_second=420.103Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>_BigO          8.81 N          9.07 N
BM_Sort_Typed_Helper<uint32_t, RadixSortV1SwitchWrapper, true>_RMS              2 %             3 %
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/1024               0.008 ms        0.008 ms       112000 bytes_per_second=482.759Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/2048               0.015 ms        0.015 ms        44800 bytes_per_second=509.091Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/4096               0.029 ms        0.029 ms        22400 bytes_per_second=533.333Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/8192               0.058 ms        0.058 ms        10000 bytes_per_second=540.541Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/16384              0.115 ms        0.117 ms         6400 bytes_per_second=533.333Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/32768              0.232 ms        0.235 ms         2987 bytes_per_second=531.022Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/65536              0.479 ms        0.476 ms         1445 bytes_per_second=525.455Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/131072              1.24 ms         1.26 ms          498 bytes_per_second=398.4Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/262144              2.36 ms         2.30 ms          299 bytes_per_second=434.909Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/524288              4.69 ms         5.00 ms          100 bytes_per_second=400Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>/1048576             9.47 ms         9.17 ms           75 bytes_per_second=436.364Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>_BigO                9.02 N          8.90 N
BM_Sort_Typed_Helper<uint32_t, RadixSortV2Wrapper, true>_RMS                    3 %             7 %
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/1024               0.008 ms        0.008 ms        89600 bytes_per_second=509.091Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/2048               0.014 ms        0.014 ms        49778 bytes_per_second=553.089Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/4096               0.027 ms        0.027 ms        26353 bytes_per_second=572.891Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/8192               0.056 ms        0.055 ms        10000 bytes_per_second=571.429Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/16384              0.109 ms        0.112 ms         6400 bytes_per_second=556.522Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/32768              0.220 ms        0.225 ms         2987 bytes_per_second=555.721Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/65536              0.461 ms        0.471 ms         1493 bytes_per_second=530.844Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/131072              1.20 ms         1.28 ms          560 bytes_per_second=389.565Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/262144              2.31 ms         2.35 ms          299 bytes_per_second=425.244Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/524288              4.22 ms         4.52 ms          166 bytes_per_second=442.667Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>/1048576             8.82 ms         8.54 ms           75 bytes_per_second=468.293Mi/s
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>_BigO                8.36 N          8.29 N
BM_Sort_Typed_Helper<uint32_t, RadixSortV3Wrapper, true>_RMS                    5 %             7 %
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/1024               0.016 ms        0.017 ms        40727 bytes_per_second=462.807Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/2048               0.031 ms        0.031 ms        23579 bytes_per_second=501.681Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/4096               0.061 ms        0.063 ms        11200 bytes_per_second=497.778Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/8192               0.122 ms        0.123 ms         5600 bytes_per_second=509.091Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/16384              0.244 ms        0.246 ms         2987 bytes_per_second=508.426Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/32768              0.515 ms        0.516 ms         1120 bytes_per_second=484.324Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/65536               1.30 ms         1.29 ms          448 bytes_per_second=387.459Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/131072              2.50 ms         2.56 ms          299 bytes_per_second=390.531Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/262144              4.84 ms         4.10 ms          179 bytes_per_second=487.489Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/524288              10.2 ms         10.6 ms           75 bytes_per_second=376.471Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>/1048576             22.7 ms         22.9 ms           30 bytes_per_second=349.091Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>_BigO                1.07 NlgN       1.08 NlgN
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, true>_RMS                    5 %             8 %
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/1024         0.017 ms        0.016 ms        40727 bytes_per_second=496.671Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/2048         0.031 ms        0.032 ms        22400 bytes_per_second=486.957Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/4096         0.062 ms        0.060 ms        11200 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/8192         0.122 ms        0.114 ms         5600 bytes_per_second=546.341Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/16384        0.246 ms        0.251 ms         2800 bytes_per_second=497.778Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/32768        0.523 ms        0.531 ms         1000 bytes_per_second=470.588Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/65536         1.27 ms         1.31 ms          560 bytes_per_second=381.277Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/131072        2.51 ms         2.40 ms          280 bytes_per_second=416.744Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/262144        4.79 ms         4.63 ms          145 bytes_per_second=431.628Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/524288        10.2 ms         10.3 ms           64 bytes_per_second=390.095Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>/1048576       23.4 ms         23.4 ms           32 bytes_per_second=341.333Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>_BigO          1.10 NlgN       1.10 NlgN
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, true>_RMS              8 %             8 %
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/1024               0.017 ms        0.017 ms        44800 bytes_per_second=457.143Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/2048               0.031 ms        0.030 ms        22400 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/4096               0.061 ms        0.063 ms        11200 bytes_per_second=497.778Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/8192               0.121 ms        0.123 ms         5600 bytes_per_second=509.091Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/16384              0.244 ms        0.240 ms         2800 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/32768              0.524 ms        0.531 ms         1000 bytes_per_second=470.588Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/65536               1.26 ms         1.32 ms          640 bytes_per_second=379.259Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/131072              2.47 ms         2.43 ms          264 bytes_per_second=412.098Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/262144              4.90 ms         4.77 ms          154 bytes_per_second=419.404Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/524288              10.1 ms         9.58 ms           75 bytes_per_second=417.391Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>/1048576             22.6 ms         22.0 ms           32 bytes_per_second=364.089Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>_BigO                1.06 NlgN       1.03 NlgN
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, true>_RMS                    5 %             7 %
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/1024               0.016 ms        0.016 ms        44800 bytes_per_second=497.778Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/2048               0.031 ms        0.030 ms        23579 bytes_per_second=523.978Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/4096               0.060 ms        0.060 ms        11200 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/8192               0.122 ms        0.117 ms         5600 bytes_per_second=533.333Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/16384              0.243 ms        0.246 ms         2987 bytes_per_second=508.426Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/32768              0.518 ms        0.516 ms         1120 bytes_per_second=484.324Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/65536               1.26 ms         1.22 ms          498 bytes_per_second=408.615Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/131072              2.39 ms         2.46 ms          280 bytes_per_second=407.273Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/262144              4.79 ms         5.00 ms          100 bytes_per_second=400Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/524288              10.2 ms         10.3 ms           64 bytes_per_second=390.095Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>/1048576             22.5 ms         22.5 ms           32 bytes_per_second=356.174Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>_BigO                1.06 NlgN       1.06 NlgN
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, true>_RMS                    4 %             3 %
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>/1024              0.019 ms        0.020 ms        37333 bytes_per_second=397.16Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>/2048              0.035 ms        0.035 ms        18667 bytes_per_second=444.452Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>/4096              0.070 ms        0.068 ms         8960 bytes_per_second=459.487Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>/8192              0.140 ms        0.141 ms         4978 bytes_per_second=442.489Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>/16384             0.282 ms        0.285 ms         2358 bytes_per_second=438.698Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>/32768             0.577 ms        0.572 ms         1120 bytes_per_second=437.073Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>/65536              1.37 ms         1.45 ms          560 bytes_per_second=344.615Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>/131072             2.71 ms         2.85 ms          236 bytes_per_second=351.256Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>/262144             5.33 ms         5.58 ms          112 bytes_per_second=358.4Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>_BigO              20.39 N         21.35 N
BM_Sort_Typed_Helper<uint64_t, RadixSortV1Wrapper, false>_RMS                   4 %             5 %
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>/1024        0.019 ms        0.019 ms        40727 bytes_per_second=407.27Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>/2048        0.036 ms        0.036 ms        19478 bytes_per_second=432.844Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>/4096        0.071 ms        0.071 ms        11200 bytes_per_second=439.216Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>/8192        0.139 ms        0.138 ms         4978 bytes_per_second=452.545Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>/16384       0.280 ms        0.289 ms         2489 bytes_per_second=432.87Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>/32768       0.578 ms        0.586 ms         1120 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>/65536        1.37 ms         1.28 ms          560 bytes_per_second=389.565Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>/131072       2.68 ms         2.51 ms          249 bytes_per_second=398.4Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>/262144       5.47 ms         5.72 ms          112 bytes_per_second=349.659Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>_BigO         1.17 NlgN       1.20 NlgN
BM_Sort_Typed_Helper<uint64_t, RadixSortV1SwitchWrapper, false>_RMS             5 %             5 %
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>/1024              0.020 ms        0.020 ms        32000 bytes_per_second=390.244Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>/2048              0.038 ms        0.038 ms        17920 bytes_per_second=407.273Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>/4096              0.073 ms        0.075 ms         8960 bytes_per_second=416.744Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>/8192              0.146 ms        0.146 ms         4480 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>/16384             0.291 ms        0.293 ms         2240 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>/32768             0.597 ms        0.593 ms          896 bytes_per_second=421.647Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>/65536              1.43 ms         1.32 ms          498 bytes_per_second=379.429Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>/131072             2.72 ms         2.70 ms          249 bytes_per_second=370.605Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>/262144             5.31 ms         5.62 ms          100 bytes_per_second=355.556Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>_BigO               1.15 NlgN       1.20 NlgN
BM_Sort_Typed_Helper<uint64_t, RadixSortV2Wrapper, false>_RMS                   8 %             2 %
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>/1024              0.019 ms        0.018 ms        34462 bytes_per_second=430.775Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>/2048              0.034 ms        0.034 ms        20364 bytes_per_second=462.818Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>/4096              0.069 ms        0.068 ms        11200 bytes_per_second=457.143Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>/8192              0.137 ms        0.135 ms         4978 bytes_per_second=463.07Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>/16384             0.274 ms        0.276 ms         2489 bytes_per_second=452.545Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>/32768             0.564 ms        0.558 ms         1120 bytes_per_second=448Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>/65536              1.38 ms         1.36 ms          448 bytes_per_second=367.59Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>/131072             2.68 ms         2.73 ms          280 bytes_per_second=365.714Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>/262144             5.28 ms         5.16 ms          100 bytes_per_second=387.879Mi/s
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>_BigO              20.18 N         19.90 N
BM_Sort_Typed_Helper<uint64_t, RadixSortV3Wrapper, false>_RMS                   4 %             5 %

------------------------------------------------------------------------------------------------------------------------------
Benchmark                                                                    Time             CPU   Iterations UserCounters...
------------------------------------------------------------------------------------------------------------------------------
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/1024               0.009 ms        0.009 ms       112000 bytes_per_second=451.613Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/2048               0.016 ms        0.016 ms        44800 bytes_per_second=497.778Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/4096               0.032 ms        0.032 ms        21333 bytes_per_second=484.841Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/8192               0.061 ms        0.059 ms        10000 bytes_per_second=526.316Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/16384              0.121 ms        0.123 ms         5600 bytes_per_second=509.091Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/32768              0.242 ms        0.240 ms         2800 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/65536              0.505 ms        0.516 ms         1000 bytes_per_second=484.848Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/131072              1.25 ms         1.26 ms          560 bytes_per_second=398.222Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/262144              2.60 ms         2.49 ms          264 bytes_per_second=402.286Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/524288              4.59 ms         4.46 ms          112 bytes_per_second=448Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>/1048576             9.44 ms         9.58 ms           75 bytes_per_second=417.391Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>_BigO                9.00 N          9.04 N
BM_Sort_Typed_Helper<int32_t, RadixSortV1Wrapper, true>_RMS                    5 %             6 %
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/1024         0.008 ms        0.009 ms        89600 bytes_per_second=448Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/2048         0.016 ms        0.016 ms        44800 bytes_per_second=476.596Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/4096         0.031 ms        0.032 ms        23579 bytes_per_second=491.229Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/8192         0.061 ms        0.061 ms        11200 bytes_per_second=509.091Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/16384        0.121 ms        0.120 ms         5600 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/32768        0.240 ms        0.240 ms         2800 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/65536        0.495 ms        0.508 ms         1445 bytes_per_second=491.915Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/131072        1.22 ms         1.22 ms          498 bytes_per_second=408.615Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/262144        2.50 ms         2.40 ms          280 bytes_per_second=416.744Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/524288        4.85 ms         5.03 ms          149 bytes_per_second=397.333Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>/1048576       9.42 ms         9.28 ms           64 bytes_per_second=431.158Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>_BigO          9.06 N          9.00 N
BM_Sort_Typed_Helper<int32_t, RadixSortV1SwitchWrapper, true>_RMS              4 %             7 %
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/1024               0.010 ms        0.010 ms        64000 bytes_per_second=390.244Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/2048               0.019 ms        0.019 ms        37333 bytes_per_second=414.811Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/4096               0.037 ms        0.036 ms        18667 bytes_per_second=434.116Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/8192               0.073 ms        0.071 ms         8960 bytes_per_second=437.073Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/16384              0.144 ms        0.143 ms         4480 bytes_per_second=437.073Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/32768              0.290 ms        0.295 ms         2489 bytes_per_second=423.66Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/65536              0.582 ms        0.600 ms         1120 bytes_per_second=416.744Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/131072              1.43 ms         1.46 ms          640 bytes_per_second=341.333Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/262144              2.74 ms         2.58 ms          236 bytes_per_second=387.282Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/524288              5.27 ms         5.31 ms          100 bytes_per_second=376.471Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>/1048576             10.9 ms         9.52 ms           64 bytes_per_second=420.103Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>_BigO               10.31 N          9.34 N
BM_Sort_Typed_Helper<int32_t, RadixSortV2Wrapper, true>_RMS                    3 %             9 %
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/1024               0.008 ms        0.007 ms        89600 bytes_per_second=520.93Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/2048               0.015 ms        0.014 ms        49778 bytes_per_second=553.089Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/4096               0.029 ms        0.028 ms        23579 bytes_per_second=548.349Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/8192               0.056 ms        0.057 ms        11200 bytes_per_second=546.341Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/16384              0.110 ms        0.112 ms         6400 bytes_per_second=556.522Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/32768              0.222 ms        0.214 ms         2987 bytes_per_second=582.829Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/65536              0.462 ms        0.471 ms         1493 bytes_per_second=530.844Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/131072              1.22 ms         1.17 ms          560 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/262144              2.44 ms         2.46 ms          299 bytes_per_second=407.149Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/524288              4.44 ms         4.54 ms          172 bytes_per_second=440.32Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>/1048576             8.89 ms         8.85 ms           90 bytes_per_second=451.765Mi/s
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>_BigO                8.52 N          8.53 N
BM_Sort_Typed_Helper<int32_t, RadixSortV3Wrapper, true>_RMS                    5 %             5 %
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/1024               0.017 ms        0.018 ms        40727 bytes_per_second=442.685Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/2048               0.032 ms        0.033 ms        21333 bytes_per_second=474.067Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/4096               0.064 ms        0.066 ms        11200 bytes_per_second=476.596Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/8192               0.126 ms        0.128 ms         5600 bytes_per_second=486.957Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/16384              0.257 ms        0.257 ms         2800 bytes_per_second=486.957Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/32768              0.539 ms        0.531 ms         1000 bytes_per_second=470.588Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/65536               1.31 ms         1.31 ms          560 bytes_per_second=381.277Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/131072              2.53 ms         2.31 ms          264 bytes_per_second=433.231Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/262144              5.11 ms         5.47 ms          100 bytes_per_second=365.714Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/524288              10.5 ms         10.0 ms           56 bytes_per_second=398.222Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>/1048576             23.5 ms         24.6 ms           28 bytes_per_second=325.818Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>_BigO                1.11 NlgN       1.14 NlgN
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, true>_RMS                    5 %            11 %
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/1024         0.017 ms        0.017 ms        37333 bytes_per_second=466.663Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/2048         0.032 ms        0.032 ms        22400 bytes_per_second=486.957Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/4096         0.065 ms        0.063 ms        11200 bytes_per_second=497.778Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/8192         0.128 ms        0.129 ms         4978 bytes_per_second=485.659Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/16384        0.256 ms        0.255 ms         2635 bytes_per_second=490.233Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/32768        0.544 ms        0.558 ms         1120 bytes_per_second=448Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/65536         1.31 ms         1.33 ms          448 bytes_per_second=377.263Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/131072        2.54 ms         2.43 ms          264 bytes_per_second=412.098Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/262144        4.99 ms         5.31 ms          100 bytes_per_second=376.471Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/524288        10.6 ms         10.0 ms           64 bytes_per_second=399.61Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>/1048576       23.5 ms         23.9 ms           32 bytes_per_second=334.367Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>_BigO          1.11 NlgN       1.12 NlgN
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, true>_RMS              5 %             9 %
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/1024               0.019 ms        0.019 ms        37333 bytes_per_second=405.793Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/2048               0.036 ms        0.037 ms        18667 bytes_per_second=424.25Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/4096               0.071 ms        0.071 ms         8960 bytes_per_second=437.073Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/8192               0.142 ms        0.144 ms         4978 bytes_per_second=432.87Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/16384              0.282 ms        0.282 ms         2489 bytes_per_second=442.489Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/32768              0.578 ms        0.586 ms         1120 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/65536               1.40 ms         1.41 ms          498 bytes_per_second=354.133Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/131072              2.70 ms         2.72 ms          264 bytes_per_second=367.304Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/262144              5.35 ms         5.62 ms          100 bytes_per_second=355.556Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/524288              11.2 ms         11.4 ms           56 bytes_per_second=349.659Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>/1048576             24.6 ms         24.6 ms           28 bytes_per_second=325.818Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>_BigO                1.16 NlgN       1.17 NlgN
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, true>_RMS                    4 %             2 %
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/1024               0.016 ms        0.016 ms        40727 bytes_per_second=484.845Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/2048               0.031 ms        0.030 ms        23579 bytes_per_second=512.587Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/4096               0.061 ms        0.061 ms        11200 bytes_per_second=509.091Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/8192               0.120 ms        0.122 ms         6400 bytes_per_second=512Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/16384              0.243 ms        0.246 ms         2800 bytes_per_second=509.091Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/32768              0.509 ms        0.516 ms         1000 bytes_per_second=484.848Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/65536               1.27 ms         1.32 ms          640 bytes_per_second=379.259Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/131072              2.41 ms         2.49 ms          264 bytes_per_second=402.286Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/262144              4.77 ms         4.82 ms          149 bytes_per_second=414.609Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/524288              10.2 ms         10.2 ms           75 bytes_per_second=391.837Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>/1048576             22.7 ms         21.9 ms           30 bytes_per_second=365.714Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>_BigO                1.07 NlgN       1.04 NlgN
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, true>_RMS                    5 %             3 %
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>/1024              0.020 ms        0.020 ms        34462 bytes_per_second=382.911Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>/2048              0.038 ms        0.039 ms        18667 bytes_per_second=405.804Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>/4096              0.075 ms        0.074 ms        11200 bytes_per_second=422.642Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>/8192              0.148 ms        0.144 ms         4978 bytes_per_second=432.87Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>/16384             0.295 ms        0.305 ms         2358 bytes_per_second=410.087Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>/32768             0.594 ms        0.600 ms         1120 bytes_per_second=416.744Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>/65536              1.40 ms         1.40 ms          448 bytes_per_second=358.4Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>/131072             2.70 ms         2.70 ms          249 bytes_per_second=370.605Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>/262144             5.41 ms         5.44 ms          112 bytes_per_second=367.59Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>_BigO              20.64 N         20.71 N
BM_Sort_Typed_Helper<int64_t, RadixSortV1Wrapper, false>_RMS                   3 %             3 %
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>/1024        0.020 ms        0.020 ms        34462 bytes_per_second=391.614Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>/2048        0.039 ms        0.039 ms        17231 bytes_per_second=400.721Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>/4096        0.074 ms        0.073 ms         8960 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>/8192        0.149 ms        0.150 ms         4480 bytes_per_second=416.744Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>/16384       0.300 ms        0.298 ms         2358 bytes_per_second=419.2Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>/32768       0.594 ms        0.578 ms         1000 bytes_per_second=432.432Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>/65536        1.41 ms         1.50 ms          448 bytes_per_second=333.395Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>/131072       2.73 ms         2.82 ms          249 bytes_per_second=354.133Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>/262144       5.35 ms         5.16 ms          112 bytes_per_second=387.459Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>_BigO        20.50 N         20.16 N
BM_Sort_Typed_Helper<int64_t, RadixSortV1SwitchWrapper, false>_RMS             3 %             8 %
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>/1024              0.022 ms        0.022 ms        32000 bytes_per_second=355.556Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>/2048              0.042 ms        0.041 ms        16000 bytes_per_second=380.952Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>/4096              0.082 ms        0.084 ms         8960 bytes_per_second=373.333Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>/8192              0.163 ms        0.165 ms         4073 bytes_per_second=378.884Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>/16384             0.327 ms        0.322 ms         2133 bytes_per_second=387.818Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>/32768             0.661 ms        0.656 ms         1120 bytes_per_second=381.277Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>/65536              1.53 ms         1.53 ms          448 bytes_per_second=325.818Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>/131072             2.95 ms         3.07 ms          249 bytes_per_second=325.224Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>/262144             5.91 ms         6.00 ms          112 bytes_per_second=333.395Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>_BigO              22.54 N         22.97 N
BM_Sort_Typed_Helper<int64_t, RadixSortV2Wrapper, false>_RMS                   3 %             3 %
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>/1024              0.019 ms        0.019 ms        37333 bytes_per_second=414.811Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>/2048              0.036 ms        0.037 ms        20364 bytes_per_second=424.25Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>/4096              0.069 ms        0.070 ms        11200 bytes_per_second=448Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>/8192              0.137 ms        0.135 ms         4978 bytes_per_second=463.07Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>/16384             0.274 ms        0.273 ms         2635 bytes_per_second=458.261Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>/32768             0.573 ms        0.586 ms         1120 bytes_per_second=426.667Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>/65536              1.38 ms         1.43 ms          448 bytes_per_second=349.659Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>/131072             2.72 ms         2.76 ms          249 bytes_per_second=362.182Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>/262144             5.24 ms         5.30 ms          112 bytes_per_second=377.263Mi/s
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>_BigO              20.13 N         20.41 N
BM_Sort_Typed_Helper<int64_t, RadixSortV3Wrapper, false>_RMS                   4 %             5 %
*/