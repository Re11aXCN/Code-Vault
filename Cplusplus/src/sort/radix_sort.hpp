// This is a header-only library that provides the implementation of radix sort algorithm in C++ 20.
// 
// <Time complexity: O( 2 * (8N or 4N or 2N) ) , Space complexity: O(N + 2 * _Bucket_size)>
// It depends on the number of bytes of the type and the _Bucket_size settings - 256 or 65536.
// 
// The code is based on the following 
// article:
// https://www.geeksforgeeks.org/radix-sort/
// video:
// https://www.bilibili.com/video/BV1Eoh8zyEG1/
//

#pragma once

#include <type_traits>
#include <vector>
#include <array>
#include <iterator>
#include <algorithm>
#include <bit>
#include <execution>
#include <omp.h>

enum class Signedness {
    Signed,     // 数据可能包含负数（默认）
    Unsigned    // 数据确定没有负数, 则可以开启此选项以提高性能，有符号类型处理速度等效于无符号类型，如果数据有负数，则排序结果可能不正确
};

enum class NaNPosition {
    //< Temp: {INFINITY, -INFINITY, NAN, 1.0f / 1.0f, -1.0f / 1.0f, std::sqrt(-1.0f), 3.14f}

    //< -nan(ind) always ahead, nan always behind
    Unhandled,  // -nan(ind) -inf -1 1 3.14 inf nan

    //< -nan(ind)/nan order is uncertain, but they are always behind/ahead of other values
    AtStart,    // nan -nan(ind) -inf -1 1 3.14 inf
    AtEnd       // -inf -1 1 3.14 inf -nan(ind) nan
};

template <typename T>
struct is_execution_policy : std::false_type {};

template <>
struct is_execution_policy<std::execution::sequenced_policy> : std::true_type {};

template <>
struct is_execution_policy<std::execution::parallel_policy> : std::true_type {};

template <typename T>
inline constexpr bool is_execution_policy_v = is_execution_policy<T>::value;

template<typename T>
struct identity_key_extractor {
    template<typename U>
    constexpr auto operator()(U&& value) const noexcept
        -> std::enable_if_t<std::is_same_v<std::decay_t<U>, T>, T>
    {
        return std::forward<U>(value);
    }
};

template<typename Class, typename Key>
struct member_key_extractor {
    Key Class::* member_ptr;

    constexpr member_key_extractor(Key Class::* ptr) noexcept : member_ptr(ptr) {}

    constexpr Key operator()(const Class& obj) const noexcept {
        return obj.*member_ptr;
    }
};

template<typename Func>
struct function_key_extractor {
    Func func;

    constexpr function_key_extractor(Func f) : func(std::move(f)) {}

    template<typename T>
    constexpr auto operator()(T&& value) const
        -> decltype(func(std::forward<T>(value)))
    {
        return func(std::forward<T>(value));
    }
};

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size, Signedness _Has_negative, NaNPosition _NaN_position>
requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
        (std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_impl(std::execution::sequenced_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
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
    
    constexpr std::size_t _Key_bits = sizeof(Key_t) << 3;
    constexpr Unsigned_t _Sign_bit_mask = Unsigned_t{ 1 } << (_Key_bits - 1);
    constexpr Unsigned_t _All_bits_mask = ~Unsigned_t{ 0 };

    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;
    [[maybe_unused]] std::size_t _NaN_count = 0;

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
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

            if constexpr (_Has_negative == Signedness::Signed && std::is_floating_point_v<Key_t>) {
                if (_Unsigned_value >> (_Key_bits - 1)) _Unsigned_value ^= _All_bits_mask;
            }

            std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;
            if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;
            ++_Bucket_count[_Byte_idx];
        }

        std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(),
            _Scanned.begin(), 0, std::plus<>{});

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            auto _Value = std::move(_Start[_Idx]);
            Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

            if constexpr (_Has_negative == Signedness::Signed && std::is_floating_point_v<Key_t>) {
                if (_Unsigned_value >> (_Key_bits - 1)) _Unsigned_value ^= _All_bits_mask;
            }

            std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;
            if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;
            _End[_Scanned[_Byte_idx]++] = std::move(_Value);
        }
        std::swap(_Start, _End);
    }

    // Signed Last Pass
    // ^^^^^^
    std::fill(_Bucket_count.begin(), _Bucket_count.end(), 0);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
    // Count the number of elements per bucket
    for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
        auto _Key = _Extractor(_Start[_Idx]);

        if constexpr (_NaN_position != NaNPosition::Unhandled && std::is_floating_point_v<Key_t>) {
            if (std::isnan(_Key)) { ++_NaN_count; continue; }
        }
        Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Key);

        if constexpr (_Has_negative == Signedness::Signed && std::is_signed_v<Key_t>) {
            if constexpr (std::is_floating_point_v<Key_t>) {
                _Unsigned_value ^= ((_Unsigned_value >> (_Key_bits - 1)) == 0) ? _Sign_bit_mask : _All_bits_mask;
            }
            else {
                _Unsigned_value ^= _Sign_bit_mask;
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
    if constexpr (_NaN_position == NaNPosition::AtStart && std::is_floating_point_v<Key_t>) {
        std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(),
            _Scanned.begin(), _NaN_count, std::plus<>{});
    }
    else {
        std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(),
            _Scanned.begin(), 0, std::plus<>{});
    }
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
        auto _Key = _Extractor(_Value);

        if constexpr (_NaN_position != NaNPosition::Unhandled && std::is_floating_point_v<Key_t>) {
            if (std::isnan(_Key)) {
                if constexpr (_NaN_position == NaNPosition::AtStart) {
                    _End[--_NaN_count] = std::move(_Value);
                }
                else if constexpr (_NaN_position == NaNPosition::AtEnd) {
                    _End[_Size - _NaN_count--] = std::move(_Value);
                }
                continue;
            }
        }

        Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Key);

        if constexpr (_Has_negative == Signedness::Signed && std::is_signed_v<Key_t>) {
            if constexpr (std::is_floating_point_v<Key_t>) {
                _Unsigned_value ^= ((_Unsigned_value >> (_Key_bits - 1)) == 0) ? _Sign_bit_mask : _All_bits_mask;
            }
            else {
                _Unsigned_value ^= _Sign_bit_mask;
            }
        }

        std::uint16_t _Byte_idx = (_Unsigned_value >> ((_Passes - 1) << _Shift)) & _Mask;

        if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

        _End[_Scanned[_Byte_idx]++] = std::move(_Value);
    }
    // Swap buffer
    std::swap(_Start, _End);

    // Note that _Buffer is a temporary variable, using copy_n not swap
    // If passes is ODD, we need to copy the data back to the original array
    // But passes is EVEN, we already have the data in the buffer
    // if (_Start != _Ptr)  std::copy_n(_Start, _Size, _Ptr); //std::copy_n(std::make_move_iterator(_Start), _Size, _Ptr);
    if constexpr (_Passes & 1) std::move(_Start, _Start + _Size, _Ptr);
}

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size, Signedness _Has_negative, NaNPosition _NaN_position>
requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
        (std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>) &&
        (_NaN_position == NaNPosition::AtStart || _NaN_position == NaNPosition::AtEnd))
void radix_sort_impl_floating_point_NaN(std::execution::parallel_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(_Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_floating_point_v<Key_t>,
        "Specialization! Key type must be an floating-point type");

    using Unsigned_t = std::make_unsigned_t<std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>>;

    Value_t* _Ptr = &*_First;
    std::size_t _Size = std::distance(_First, _Last);
    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
    constexpr std::uint8_t _Shift = _Bucket_size == 256U ? 3 : 4;
    constexpr std::uint16_t _Mask = _Bucket_size - 1;

    constexpr std::size_t _Key_bits = sizeof(Key_t) << 3;
    constexpr Unsigned_t _Sign_bit_mask = Unsigned_t{ 1 } << (_Key_bits - 1);
    constexpr Unsigned_t _All_bits_mask = ~Unsigned_t{ 0 };

    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;

    const std::int32_t _Hardware_concurrency = omp_get_num_procs();

    const std::size_t _Min_chunk_size = 1024;
    const std::int32_t _Actual_threads = std::max(1,
        std::min(_Hardware_concurrency, static_cast<std::int32_t>(_Size / _Min_chunk_size)));

    const std::size_t _Chunk = (_Size + _Actual_threads - 1) / _Actual_threads;

    std::vector<std::size_t> _Local_data(_Actual_threads * _Bucket_size * 2);

    auto _Func_get_local_buckets = [&](int _Thread_id) -> std::size_t* {
        return _Local_data.data() + _Thread_id * _Bucket_size * 2;
        };

    auto _Func_get_local_offsets = [&](int _Thread_id) -> std::size_t* {
        return _Local_data.data() + _Thread_id * _Bucket_size * 2 + _Bucket_size;
        };

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
        {
            std::fill(_Local_data.begin(), _Local_data.end(), 0);

#pragma omp parallel num_threads(_Actual_threads)
            {
                const int _Thread_id = omp_get_thread_num();
                std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread_id);

                const std::size_t _Start_idx = _Thread_id * _Chunk;
                const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                    Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                    if constexpr (_Has_negative == Signedness::Signed) {
                        if (_Unsigned_value >> (_Key_bits - 1)) _Unsigned_value ^= _All_bits_mask;
                    }

                    std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

                    if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                    ++_Local_buckets[_Byte_idx];
                }
            }
        }

        {
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

        {
#pragma omp parallel num_threads(_Actual_threads)
            {
                const int _Thread_id = omp_get_thread_num();
                std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread_id);

                const std::size_t _Start_idx = _Thread_id * _Chunk;
                const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                    auto _Value = std::move(_Start[_Idx]);
                    Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                    if constexpr (_Has_negative == Signedness::Signed) {
                        if (_Unsigned_value >> (_Key_bits - 1)) _Unsigned_value ^= _All_bits_mask;
                    }

                    std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;
                    if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;
                    _End[_Local_offsets[_Byte_idx]++] = std::move(_Value);
                }
            }
        }
        std::swap(_Start, _End);
    }

    std::size_t _NaN_count = 0;
    std::vector<std::size_t> _Local_NaN_counts(_Actual_threads, 0);

    // Signed Last Pass
    // ^^^^^^
    {   
        std::fill(_Local_data.begin(), _Local_data.end(), 0);

#pragma omp parallel num_threads(_Actual_threads)
        {
            const int _Thread_id = omp_get_thread_num();
            std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread_id);

            const std::size_t _Start_idx = _Thread_id * _Chunk;
            const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                auto _Key = _Extractor(_Start[_Idx]);

                if (std::isnan(_Key)) { ++_Local_NaN_counts[_Thread_id]; continue; }

                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Key);
                if constexpr (_Has_negative == Signedness::Signed) {
                    _Unsigned_value ^= ((_Unsigned_value >> (_Key_bits - 1)) == 0) ? _Sign_bit_mask : _All_bits_mask;
                }
                std::uint16_t _Byte_idx = (_Unsigned_value >> ((_Passes - 1) << _Shift)) & _Mask;
                if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;
                ++_Local_buckets[_Byte_idx];
            }
        }

        _NaN_count = std::accumulate(_Local_NaN_counts.begin(), _Local_NaN_counts.end(), 0);
    }

    {
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

        std::size_t _Running_sum = 0;
        if constexpr (_NaN_position == NaNPosition::AtStart) {
            _Running_sum = _NaN_count;
        }

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

    {
        std::vector<std::size_t> _Thread_NaN_offsets(_Actual_threads);
        if constexpr (_NaN_position == NaNPosition::AtStart) {
            std::size_t _NaN_running_sum = 0;
#if defined(__clang__)
#pragma clang loop vectorize(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::int32_t _Thread = 0; _Thread < _Actual_threads; ++_Thread) {
                _Thread_NaN_offsets[_Thread] = _NaN_running_sum;
                _NaN_running_sum += _Local_NaN_counts[_Thread];
            }
        }
        else if constexpr (_NaN_position == NaNPosition::AtEnd) {
            std::size_t _NaN_running_sum = _Size - _NaN_count;
#if defined(__clang__)
#pragma clang loop vectorize(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::int32_t _Thread = 0; _Thread < _Actual_threads; ++_Thread) {
                _Thread_NaN_offsets[_Thread] = _NaN_running_sum;
                _NaN_running_sum += _Local_NaN_counts[_Thread];
            }
        }

#pragma omp parallel num_threads(_Actual_threads)
        {
            const int _Thread_id = omp_get_thread_num();
            std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread_id);
            std::size_t _Thread_NaN_offset = _Thread_NaN_offsets[_Thread_id];

            const std::size_t _Start_idx = _Thread_id * _Chunk;
            const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                auto _Value = std::move(_Start[_Idx]);
                auto _Key = _Extractor(_Value);

                if (std::isnan(_Key)) {
                    if constexpr (_NaN_position == NaNPosition::AtStart) {
                        _End[_Thread_NaN_offset++] = std::move(_Value);
                    }
                    else if constexpr (_NaN_position == NaNPosition::AtEnd) {
                        _End[_Thread_NaN_offset++] = std::move(_Value);
                    }
                    continue;
                }

                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Key);
                if constexpr (_Has_negative == Signedness::Signed) {
                    _Unsigned_value ^= ((_Unsigned_value >> (_Key_bits - 1)) == 0) ? _Sign_bit_mask : _All_bits_mask;
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

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size, Signedness _Has_negative, NaNPosition _NaN_position>
requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
        (std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_impl(std::execution::parallel_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(_Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool> && sizeof(Key_t) >= 4,
        "Key type must be an arithmetic type (not bool) and at least 4 bytes. If you want to sort 8bits or 16bits, please using sequential_execution_policy.");
    
    if constexpr (std::is_floating_point_v<Key_t> &&
        (_NaN_position == NaNPosition::AtStart || _NaN_position == NaNPosition::AtEnd)) {
        radix_sort_impl_floating_point_NaN<ContigIter, Compare, KeyExtractor,
            _Bucket_size, _Has_negative, _NaN_position>(std::execution::par, _First, _Last, std::forward<KeyExtractor>(_Extractor));
        return;
    }

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

    constexpr std::size_t _Key_bits = sizeof(Key_t) << 3;
    constexpr Unsigned_t _Sign_bit_mask = Unsigned_t{ 1 } << (_Key_bits - 1);
    constexpr Unsigned_t _All_bits_mask = ~Unsigned_t{ 0 };

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
        {
            std::fill(_Local_data.begin(), _Local_data.end(), 0);

#pragma omp parallel num_threads(_Actual_threads)
            {
                const int _Thread_id = omp_get_thread_num();
                std::size_t* _Local_buckets = _Func_get_local_buckets(_Thread_id);

                const std::size_t _Start_idx = _Thread_id * _Chunk;
                const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                    Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                    if constexpr (_Has_negative == Signedness::Signed && std::is_floating_point_v<Key_t>) {
                        if (_Unsigned_value >> (_Key_bits - 1)) _Unsigned_value ^= _All_bits_mask;
                    }

                    std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

                    if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                    ++_Local_buckets[_Byte_idx];
                }
            }
        }

        {
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

        {
#pragma omp parallel num_threads(_Actual_threads)
            {
                const int _Thread_id = omp_get_thread_num();
                std::size_t* _Local_offsets = _Func_get_local_offsets(_Thread_id);

                const std::size_t _Start_idx = _Thread_id * _Chunk;
                const std::size_t _End_idx = std::min(_Size, (_Thread_id + 1) * _Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::size_t _Idx = _Start_idx; _Idx < _End_idx; ++_Idx) {
                    auto _Value = std::move(_Start[_Idx]);
                    Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                    if constexpr (_Has_negative == Signedness::Signed && std::is_floating_point_v<Key_t>) {
                        if (_Unsigned_value >> (_Key_bits - 1)) _Unsigned_value ^= _All_bits_mask;
                    }

                    std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;
                    if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;
                    _End[_Local_offsets[_Byte_idx]++] = std::move(_Value);
                }
            }
        }
        std::swap(_Start, _End);
    }

    // Signed Last Pass
    // ^^^^^^
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

                if constexpr (_Has_negative == Signedness::Signed && std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        _Unsigned_value ^= ((_Unsigned_value >> (_Key_bits - 1)) == 0) ? _Sign_bit_mask : _All_bits_mask;
                    }
                    else {
                        _Unsigned_value ^= _Sign_bit_mask;
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

                if constexpr (_Has_negative == Signedness::Signed && std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        _Unsigned_value ^= ((_Unsigned_value >> (_Key_bits - 1)) == 0) ? _Sign_bit_mask : _All_bits_mask;
                    }
                    else {
                        _Unsigned_value ^= _Sign_bit_mask;
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

// 带键提取器的完整版本
template<typename ExecutionPolicy, typename ContigIter, typename Compare, typename KeyExtractor,
    std::uint32_t BucketSize = 256U,
    Signedness HasNegative = Signedness::Signed,
    NaNPosition NaNPosition = NaNPosition::Unhandled,
    std::enable_if_t<is_execution_policy_v<std::decay_t<ExecutionPolicy>>, int> = 0>
void radix_sort(ExecutionPolicy policy, ContigIter first, ContigIter last, Compare comp, KeyExtractor&& extractor)
{
    radix_sort_impl<ContigIter, Compare, KeyExtractor, BucketSize, HasNegative, NaNPosition>(policy, first, last, std::forward<KeyExtractor>(extractor));
}

// 基础版本：接受比较器和执行策略
template<typename ExecutionPolicy, typename ContigIter, typename Compare>
void radix_sort(ExecutionPolicy&& policy, ContigIter first, ContigIter last, Compare comp)
{
    using Value_t = std::iter_value_t<ContigIter>;
    radix_sort(std::forward<ExecutionPolicy>(policy), first, last, comp, identity_key_extractor<Value_t>{});
}

// 基础版本：只接受比较器（默认执行策略）
template<typename ContigIter, typename Compare>
void radix_sort(ContigIter first, ContigIter last, Compare comp)
{
    radix_sort(std::execution::seq, first, last, comp);
}

// 执行策略版本：只接受迭代器范围（默认升序）
template<typename ExecutionPolicy, typename ContigIter>
void radix_sort(ExecutionPolicy&& policy, ContigIter first, ContigIter last)
{
    radix_sort(std::forward<ExecutionPolicy>(policy), first, last, std::less<>{});
}

// 简化版本：只接受迭代器范围（默认升序，默认执行策略）
template<typename ContigIter>
void radix_sort(ContigIter first, ContigIter last)
{
    radix_sort(first, last, std::less<>{});
}

// 辅助函数：按成员排序
template<typename ExecutionPolicy, typename ContigIter, typename KeyType, typename Compare = std::less<>>
void radix_sort_by_member(ExecutionPolicy&& policy, ContigIter first, ContigIter last,
    KeyType (std::iter_value_t<ContigIter>::* member_ptr),
    Compare comp = {})
{
    using Value_t = std::iter_value_t<ContigIter>;
    radix_sort(std::forward<ExecutionPolicy>(policy), first, last, comp, member_key_extractor<Value_t, KeyType>{member_ptr});
}

template<typename ContigIter, typename KeyType, typename Compare = std::less<>>
void radix_sort_by_member(ContigIter first, ContigIter last,
    KeyType (std::iter_value_t<ContigIter>::* member_ptr),
    Compare comp = {})
{
    using Value_t = std::iter_value_t<ContigIter>;
    radix_sort_by_member(std::execution::seq, first, last, member_ptr, comp);
}

// 辅助函数：按函数排序
template<typename ExecutionPolicy, typename ContigIter, typename Func, typename Compare = std::less<>>
void radix_sort_by(ExecutionPolicy&& policy, ContigIter first, ContigIter last, Func&& func, Compare comp = {})
{
    radix_sort(std::forward<ExecutionPolicy>(policy), first, last, comp, function_key_extractor<std::decay_t<Func>>{std::forward<Func>(func)});
}

template<typename ContigIter, typename Func, typename Compare = std::less<>>
void radix_sort_by(ContigIter first, ContigIter last, Func&& func, Compare comp = {})
{
    radix_sort_by(std::execution::seq, first, last, std::forward<Func>(func), comp);
}
/*
#include "tools/ticktock.h"
#include "tools/generator.h"
template<typename T, typename Compare>
void test_radix_sort(const std::string& type_name, Compare comp, const std::string& order_name) {
    std::cout << "测试类型: " << type_name << ", 排序方式: " << order_name << std::endl;

    // 生成测试数据
    auto data = generate_vec_data<T>(1000,
        std::numeric_limits<T>::lowest() / 2 + std::numeric_limits<T>::lowest() / 4,
        std::numeric_limits<T>::max() / 2 + std::numeric_limits<T>::max() / 4);

    auto data_copy = data; // 备份用于验证

    // 使用基数排序
    radix_sort(data.begin(), data.end(), comp);

    // 检查排序是否正确
    bool is_sorted_correctly = std::is_sorted(data.begin(), data.end(), comp);

    std::cout << "  基数排序结果: " << (is_sorted_correctly ? "正确" : "错误") << std::endl;

    // 使用std::sort验证
    std::sort(data_copy.begin(), data_copy.end(), comp);
    bool matches_std_sort = (data == data_copy);

    std::cout << "  与std::sort结果一致: " << (matches_std_sort ? "是" : "否") << std::endl;
    std::cout << "  ---------------------------------" << std::endl;
}

// 特殊处理uint8_t/int8_t（因为numeric_limits的问题）
template<>
void test_radix_sort<uint8_t, std::less<uint8_t>>(const std::string& type_name, std::less<uint8_t> comp, const std::string& order_name) {
    std::cout << "测试类型: " << type_name << ", 排序方式: " << order_name << std::endl;

    auto data = generate_vec_data<uint8_t>(1000, 0, 255);
    auto data_copy = data;

    radix_sort(data.begin(), data.end(), std::less<>{});

    bool is_sorted_correctly = std::is_sorted(data.begin(), data.end(), std::less<>{});
    std::cout << "  基数排序结果: " << (is_sorted_correctly ? "正确" : "错误") << std::endl;

    std::sort(data_copy.begin(), data_copy.end(), std::less<>{});
    bool matches_std_sort = (data == data_copy);
    std::cout << "  与std::sort结果一致: " << (matches_std_sort ? "是" : "否") << std::endl;
    std::cout << "  ---------------------------------" << std::endl;
}

template<>
void test_radix_sort<int8_t, std::less<int8_t>>(const std::string& type_name, std::less<int8_t> comp, const std::string& order_name) {
    std::cout << "测试类型: " << type_name << ", 排序方式: " << order_name << std::endl;

    auto data = generate_vec_data<int8_t>(1000, -128, 127);
    auto data_copy = data;

    radix_sort(data.begin(), data.end(), std::less<>{});

    bool is_sorted_correctly = std::is_sorted(data.begin(), data.end(), std::less<>{});
    std::cout << "  基数排序结果: " << (is_sorted_correctly ? "正确" : "错误") << std::endl;

    std::sort(data_copy.begin(), data_copy.end(), std::less<>{});
    bool matches_std_sort = (data == data_copy);
    std::cout << "  与std::sort结果一致: " << (matches_std_sort ? "是" : "否") << std::endl;
    std::cout << "  ---------------------------------" << std::endl;
}

template<>
void test_radix_sort<uint8_t, std::greater<uint8_t>>(const std::string& type_name, std::greater<uint8_t> comp, const std::string& order_name) {
    std::cout << "测试类型: " << type_name << ", 排序方式: " << order_name << std::endl;

    auto data = generate_vec_data<uint8_t>(1000, 0, 255);
    auto data_copy = data;

    radix_sort(data.begin(), data.end(), std::greater<>{});

    bool is_sorted_correctly = std::is_sorted(data.begin(), data.end(), std::greater<>{});
    std::cout << "  基数排序结果: " << (is_sorted_correctly ? "正确" : "错误") << std::endl;

    std::sort(data_copy.begin(), data_copy.end(), std::greater<>{});
    bool matches_std_sort = (data == data_copy);
    std::cout << "  与std::sort结果一致: " << (matches_std_sort ? "是" : "否") << std::endl;
    std::cout << "  ---------------------------------" << std::endl;
}

template<>
void test_radix_sort<int8_t, std::greater<int8_t>>(const std::string& type_name, std::greater<int8_t> comp, const std::string& order_name) {
    std::cout << "测试类型: " << type_name << ", 排序方式: " << order_name << std::endl;

    auto data = generate_vec_data<int8_t>(1000, -128, 127);
    auto data_copy = data;

    radix_sort(data.begin(), data.end(), std::greater<>{});

    bool is_sorted_correctly = std::is_sorted(data.begin(), data.end(), std::greater<>{});
    std::cout << "  基数排序结果: " << (is_sorted_correctly ? "正确" : "错误") << std::endl;

    std::sort(data_copy.begin(), data_copy.end(), std::greater<>{});
    bool matches_std_sort = (data == data_copy);
    std::cout << "  与std::sort结果一致: " << (matches_std_sort ? "是" : "否") << std::endl;
    std::cout << "  ---------------------------------" << std::endl;
}

int radix_sort_test() {
    std::cout.setf(std::ios_base::boolalpha);
    std::cout << "开始基数排序测试..." << std::endl;
    std::cout << "========================================" << std::endl;

    // 测试 int8_t
    {
        test_radix_sort<int8_t>("int8_t", std::less<>{}, "升序");
        test_radix_sort<int8_t>("int8_t", std::greater<>{}, "降序");
    }

    // 测试 uint8_t
    {
        test_radix_sort<uint8_t>("uint8_t", std::less<>{}, "升序");
        test_radix_sort<uint8_t>("uint8_t", std::greater<>{}, "降序");
    }

    // 测试 int32_t
    {
        test_radix_sort<int32_t>("int32_t", std::less<>{}, "升序");
        test_radix_sort<int32_t>("int32_t", std::greater<>{}, "降序");
    }

    // 测试 uint32_t
    {
        test_radix_sort<uint32_t>("uint32_t", std::less<>{}, "升序");
        test_radix_sort<uint32_t>("uint32_t", std::greater<>{}, "降序");
    }

    // 测试 int64_t
    {
        test_radix_sort<int64_t>("int64_t", std::less<>{}, "升序");
        test_radix_sort<int64_t>("int64_t", std::greater<>{}, "降序");
    }

    // 测试 uint64_t
    {
        test_radix_sort<uint64_t>("uint64_t", std::less<>{}, "升序");
        test_radix_sort<uint64_t>("uint64_t", std::greater<>{}, "降序");
    }

    // 测试 float
    {
        test_radix_sort<float>("float", std::less<>{}, "升序");
        test_radix_sort<float>("float", std::greater<>{}, "降序");
    }

    // 测试 double
    {
        test_radix_sort<double>("double", std::less<>{}, "升序");
        test_radix_sort<double>("double", std::greater<>{}, "降序");
    }

    std::cout << "========================================" << std::endl;
    std::cout << "基数排序测试完成!" << std::endl;

    struct Point {
        int x;
        float y;
    };

    auto print_points = [](const std::vector<Point>& points) -> void {
        for (const auto& p : points) {
            std::cout << "(" << p.x << ", " << p.y << ") ";
        }
        std::cout << std::endl;
        };
    std::vector<Point> points = { {2, 0.0f}, {-1, 3.0f}, {3, -4.1f}, {0, 1.5f} };

    std::cout << "Original: ";
    print_points(points);

    // 方法1: 使用成员指针按x排序
    radix_sort_by_member(points.begin(), points.end(), &Point::x);
    std::cout << "Sorted by x: ";
    print_points(points);

    // 方法2: 使用成员指针按y排序
    radix_sort_by_member(points.begin(), points.end(), &Point::y);
    std::cout << "Sorted by y: ";
    print_points(points);

    // 方法3: 使用lambda表达式按x的绝对值排序
    radix_sort_by(points.begin(), points.end(), [](const Point& p) { return std::abs(p.x); });
    std::cout << "Sorted by |x|: ";
    print_points(points);

    // 方法4: 使用lambda表达式按复杂条件排序
    radix_sort_by(points.begin(), points.end(), [](const Point& p) { return p.x + p.y; });
    std::cout << "Sorted by x+y: ";
    print_points(points);

    const size_t count = 1 << 20; // 2的20次方 = 1,048,576

    std::vector<float> randomNumbers; //{ 3.14, -2.71, 0.0, 1.41, -1.0 };
    randomNumbers.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-100.0f, 100.0f);

    for (size_t i = 0; i < count; ++i) {
        randomNumbers.push_back(dis(gen));
    }
    std::vector<float> temp1(randomNumbers);
    TICK(_1);
    radix_sort(std::execution::seq, randomNumbers.begin(), randomNumbers.end());
    TOCK(_1);

    TICK(_2);
    radix_sort(std::execution::par, temp1.begin(), temp1.end(), std::greater<>{});
    TOCK(_2);

    std::cout << std::boolalpha << std::is_sorted(randomNumbers.begin(), randomNumbers.end()) << std::endl;
    std::cout << std::boolalpha << std::is_sorted(temp1.begin(), temp1.end(), std::greater<>{}) << std::endl;

    std::cout << std::endl;
    return 0;
}
*/