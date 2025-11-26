
template<typename RanIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_impl_unrolled(parallel_execution_policy&&, RanIter first, RanIter last, KeyExtractor _Extractor)
{
    using ValueType = typename std::iterator_traits<RanIter>::value_type;
    using _Key_t = std::decay_t<decltype(_Extractor(std::declval<ValueType>()))>;
    static_assert(std::is_arithmetic_v<_Key_t> && !std::is_same_v<_Key_t, bool>,
        "Key type must be an arithmetic type (not bool)");

    using _Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<_Key_t>,
        std::conditional_t<sizeof(_Key_t) == 4, std::uint32_t, std::uint64_t>,
        _Key_t>
    >;
    auto [_Ptr, _Size] = unwrap_iterator(first, last);
    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(_Key_t) : sizeof(_Key_t) >> 1;
    constexpr std::uint16_t _Mask = _Bucket_size - 1;
    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;

    std::uint8_t _Hardware_concurrency = static_cast<std::uint8_t>(omp_get_num_procs());
    std::size_t _Chunk = (_Size + _Hardware_concurrency - 1) / _Hardware_concurrency;

    std::array<std::size_t, _Bucket_size> _Bucket_count{};
    std::vector<std::size_t> _Processor_local_buckets(_Bucket_size * _Hardware_concurrency);

    std::vector<ValueType> _Buffer(_Size);
    ValueType* _Start = _Ptr;
    ValueType* _End = _Buffer.data();

    // Helper function for key extraction and transformation
    auto extract_and_transform = [&](const auto& value) -> _Unsigned_t {
        _Unsigned_t _Unsigned_value;

        if constexpr (std::is_floating_point_v<_Key_t>) {
            _Unsigned_value = std::bit_cast<_Unsigned_t>(_Extractor(value));
            if constexpr (sizeof(_Key_t) == 4)
                _Unsigned_value ^= (_Unsigned_value & 0x80000000) ? 0xFFFFFFFF : 0x80000000;
            else
                _Unsigned_value ^= (_Unsigned_value & 0x8000000000000000) ? 0xFFFFFFFFFFFFFFFF : 0x8000000000000000;
        }
        else {
            _Unsigned_value = std::bit_cast<_Unsigned_t>(_Extractor(value));
        }
        return _Unsigned_value;
        };

    // Helper function for scattering elements
    auto scatter_element = [&](ValueType value, std::size_t* _Plb_ptr, std::uint8_t _Pass, ValueType* _Dest) {
        _Unsigned_t _Unsigned_value = extract_and_transform(value);
        std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass * 8)) & _Mask;

        if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;
        if constexpr (std::is_signed_v<_Key_t> && !std::is_floating_point_v<_Key_t>) {
            if (_Pass == _Passes - 1) _Byte_idx ^= _Bucket_size >> 1;
        }

        _Dest[_Plb_ptr[_Byte_idx]++] = value;
        };

    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
        // PHASE 1: Parallel counting with loop unrolling
#pragma omp parallel for schedule(static, 1)
        for (std::int8_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;

            // Step 1: Initialize bucket counts with loop unrolling (4x unroll)
            std::size_t _Idx = 0;
            // Process 4 buckets at a time for better cache utilization
            for (; _Idx + 3 < _Bucket_size; _Idx += 4) {
                _Plb_ptr[_Idx] = 0;
                _Plb_ptr[_Idx + 1] = 0;
                _Plb_ptr[_Idx + 2] = 0;
                _Plb_ptr[_Idx + 3] = 0;
            }
            // Handle remaining buckets
            for (; _Idx < _Bucket_size; ++_Idx) {
                _Plb_ptr[_Idx] = 0;
            }

            // Step 2: Count elements with loop unrolling
            std::size_t _Start_idx = _Core * _Chunk;
            std::size_t _End_idx = std::min(_Size, (_Core + 1) * _Chunk);
            _Idx = _Start_idx;

            // Process 4 elements at a time for better instruction-level parallelism
            for (; _Idx + 3 < _End_idx; _Idx += 4) {
                // Element 1
                _Unsigned_t _Unsigned_value0 = extract_and_transform(_Start[_Idx]);
                std::uint16_t _Byte_idx0 = (_Unsigned_value0 >> (_Pass * 8)) & _Mask;
                if constexpr (_Is_Descending) _Byte_idx0 = _Mask - _Byte_idx0;
                if constexpr (std::is_signed_v<_Key_t> && !std::is_floating_point_v<_Key_t>) {
                    if (_Pass == _Passes - 1) _Byte_idx0 ^= _Bucket_size >> 1;
                }
                ++_Plb_ptr[_Byte_idx0];

                // Element 2
                _Unsigned_t _Unsigned_value1 = extract_and_transform(_Start[_Idx + 1]);
                std::uint16_t _Byte_idx1 = (_Unsigned_value1 >> (_Pass * 8)) & _Mask;
                if constexpr (_Is_Descending) _Byte_idx1 = _Mask - _Byte_idx1;
                if constexpr (std::is_signed_v<_Key_t> && !std::is_floating_point_v<_Key_t>) {
                    if (_Pass == _Passes - 1) _Byte_idx1 ^= _Bucket_size >> 1;
                }
                ++_Plb_ptr[_Byte_idx1];

                // Element 3
                _Unsigned_t _Unsigned_value2 = extract_and_transform(_Start[_Idx + 2]);
                std::uint16_t _Byte_idx2 = (_Unsigned_value2 >> (_Pass * 8)) & _Mask;
                if constexpr (_Is_Descending) _Byte_idx2 = _Mask - _Byte_idx2;
                if constexpr (std::is_signed_v<_Key_t> && !std::is_floating_point_v<_Key_t>) {
                    if (_Pass == _Passes - 1) _Byte_idx2 ^= _Bucket_size >> 1;
                }
                ++_Plb_ptr[_Byte_idx2];

                // Element 4
                _Unsigned_t _Unsigned_value3 = extract_and_transform(_Start[_Idx + 3]);
                std::uint16_t _Byte_idx3 = (_Unsigned_value3 >> (_Pass * 8)) & _Mask;
                if constexpr (_Is_Descending) _Byte_idx3 = _Mask - _Byte_idx3;
                if constexpr (std::is_signed_v<_Key_t> && !std::is_floating_point_v<_Key_t>) {
                    if (_Pass == _Passes - 1) _Byte_idx3 ^= _Bucket_size >> 1;
                }
                ++_Plb_ptr[_Byte_idx3];
            }

            // Step 3: Process remaining elements (tail handling)
            for (; _Idx < _End_idx; ++_Idx) {
                _Unsigned_t _Unsigned_value = extract_and_transform(_Start[_Idx]);
                std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass * 8)) & _Mask;
                if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;
                if constexpr (std::is_signed_v<_Key_t> && !std::is_floating_point_v<_Key_t>) {
                    if (_Pass == _Passes - 1) _Byte_idx ^= _Bucket_size >> 1;
                }
                ++_Plb_ptr[_Byte_idx];
            }
        }

        // PHASE 2: Calculate prefix sums with optimized memory access
        // Step 1: Compute exclusive prefix scan across threads
        std::size_t _Temp_sum{ 0 };
        for (std::size_t _Bucket_idx = 0; _Bucket_idx < _Bucket_size; ++_Bucket_idx) {
            std::size_t _Temp_sum_local{ 0 };
            std::uint8_t _Core = 0;

            // Process 4 threads at a time for better cache locality
            for (; _Core + 3 < _Hardware_concurrency; _Core += 4) {
                std::size_t _Idx_local0 = _Bucket_size * _Core + _Bucket_idx;
                std::size_t _Idx_local1 = _Bucket_size * (_Core + 1) + _Bucket_idx;
                std::size_t _Idx_local2 = _Bucket_size * (_Core + 2) + _Bucket_idx;
                std::size_t _Idx_local3 = _Bucket_size * (_Core + 3) + _Bucket_idx;

                std::size_t _Temp_count_local0 = _Processor_local_buckets[_Idx_local0];
                std::size_t _Temp_count_local1 = _Processor_local_buckets[_Idx_local1];
                std::size_t _Temp_count_local2 = _Processor_local_buckets[_Idx_local2];
                std::size_t _Temp_count_local3 = _Processor_local_buckets[_Idx_local3];

                _Processor_local_buckets[_Idx_local0] = _Temp_sum_local;
                _Temp_sum_local += _Temp_count_local0;

                _Processor_local_buckets[_Idx_local1] = _Temp_sum_local;
                _Temp_sum_local += _Temp_count_local1;

                _Processor_local_buckets[_Idx_local2] = _Temp_sum_local;
                _Temp_sum_local += _Temp_count_local2;

                _Processor_local_buckets[_Idx_local3] = _Temp_sum_local;
                _Temp_sum_local += _Temp_count_local3;
            }

            // Handle remaining threads
            for (; _Core < _Hardware_concurrency; ++_Core) {
                std::size_t _Idx_local = _Bucket_size * _Core + _Bucket_idx;
                std::size_t _Temp_count_local = _Processor_local_buckets[_Idx_local];
                _Processor_local_buckets[_Idx_local] = _Temp_sum_local;
                _Temp_sum_local += _Temp_count_local;
            }

            _Bucket_count[_Bucket_idx] = _Temp_sum;
            _Temp_sum += _Temp_sum_local;
        }

        // PHASE 3: Parallel scattering with loop unrolling
#pragma omp parallel for schedule(static, 1)
        for (std::int8_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            std::size_t* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;

            // Step 1: Update local offsets with global prefix (4x unroll)
            std::size_t _Bucket_idx = 0;
            for (; _Bucket_idx + 3 < _Bucket_size; _Bucket_idx += 4) {
                _Plb_ptr[_Bucket_idx] += _Bucket_count[_Bucket_idx];
                _Plb_ptr[_Bucket_idx + 1] += _Bucket_count[_Bucket_idx + 1];
                _Plb_ptr[_Bucket_idx + 2] += _Bucket_count[_Bucket_idx + 2];
                _Plb_ptr[_Bucket_idx + 3] += _Bucket_count[_Bucket_idx + 3];
            }
            // Handle remaining buckets
            for (; _Bucket_idx < _Bucket_size; ++_Bucket_idx) {
                _Plb_ptr[_Bucket_idx] += _Bucket_count[_Bucket_idx];
            }

            // Step 2: Scatter elements with loop unrolling
            std::size_t _Start_idx = _Core * _Chunk;
            std::size_t _End_idx = std::min(_Size, (_Core + 1) * _Chunk);
            std::size_t _Idx = _Start_idx;

            // Process 4 elements at a time
            for (; _Idx + 3 < _End_idx; _Idx += 4) {
                scatter_element(_Start[_Idx], _Plb_ptr, _Pass, _End);
                scatter_element(_Start[_Idx + 1], _Plb_ptr, _Pass, _End);
                scatter_element(_Start[_Idx + 2], _Plb_ptr, _Pass, _End);
                scatter_element(_Start[_Idx + 3], _Plb_ptr, _Pass, _End);
            }

            // Step 3: Process remaining elements
            for (; _Idx < _End_idx; ++_Idx) {
                scatter_element(_Start[_Idx], _Plb_ptr, _Pass, _End);
            }
        }

        // PHASE 4: Swap buffers for next pass
        std::swap(_Start, _End);
    }
}

/// <summary>
/// 版本1特点：
/// 并行策略：使用#pragma omp parallel for schedule(static, 1)，每个线程处理固定大小的块
/// 计数阶段：每个线程独立统计本地桶计数
/// 前缀和计算：串行执行，双重循环计算全局前缀
/// 散射阶段：每个线程独立散射，使用本地偏移量
/// 
/// 计数阶段：每个线程写入独立的本地桶区域
/// 散射阶段：直接使用更新后的本地偏移量
/// 
/// </summary>
template<typename RanIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_impl_v1(parallel_execution_policy&&, RanIter first, RanIter last, KeyExtractor _Extractor)
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
#pragma clang loop unroll_count(4)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 4
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

/// <summary>
/// 版本2特点：
/// 并行策略：使用显式并行区域#pragma omp parallel，更精细的线程控制
/// 计数阶段：类似版本1，但计数后重置桶计数
/// 前缀和计算：三阶段方法（全局归约→全局前缀→本地偏移）
/// 散射阶段：使用预计算的本地偏移量
/// 
/// 额外分配了_Local_offsets数组用于存储每个线程的散射偏移
/// 内存使用量更高
/// 
/// </summary>
template<typename RanIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_impl_v2(parallel_execution_policy&&, RanIter first, RanIter last, KeyExtractor _Extractor)
{
    using ValueType = typename std::iterator_traits<RanIter>::value_type;
    using _Key_t = std::decay_t<decltype(_Extractor(std::declval<ValueType>()))>;
    static_assert(std::is_arithmetic_v<_Key_t> && !std::is_same_v<_Key_t, bool>,
        "Key type must be an arithmetic type (not bool)");

    using _Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<_Key_t>,
        std::conditional_t<sizeof(_Key_t) == 4, std::uint32_t, std::uint64_t>,
        _Key_t>
    >;
    auto [_Ptr, _Size] = unwrap_iterator(first, last);
    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(_Key_t) : sizeof(_Key_t) >> 1;
    constexpr std::uint16_t _Mask = _Bucket_size - 1;
    constexpr bool _Is_Descending = std::is_same_v<Compare, std::greater<>>;

    const std::uint8_t _Hardware_concurrency = omp_get_num_procs();
    const std::size_t _Chunk = (_Size + _Hardware_concurrency - 1) / _Hardware_concurrency;

    std::array<std::size_t, _Bucket_size> _Global_bucket_count{};
    std::vector<std::size_t> _Local_buckets(_Bucket_size * _Hardware_concurrency);

    std::vector<ValueType> _Buffer(_Size);
    ValueType* _Start = _Ptr;
    ValueType* _End = _Buffer.data();

    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
        // Reset counts
        std::fill(_Global_bucket_count.begin(), _Global_bucket_count.end(), 0);
        std::fill(_Local_buckets.begin(), _Local_buckets.end(), 0);

        // Phase 1: Local counting
#pragma omp parallel num_threads(_Hardware_concurrency)
        {
            const int _Thread_id = omp_get_thread_num();
            std::size_t* _Local_bucket = _Local_buckets.data() + _Thread_id * _Bucket_size;

#pragma omp for schedule(static)
            for (std::int64_t _Idx = 0; _Idx < _Size; ++_Idx) {
                _Unsigned_t _Unsigned_value;

                if constexpr (std::is_floating_point_v<_Key_t>) {
                    _Unsigned_value = std::bit_cast<_Unsigned_t>(_Extractor(_Start[_Idx]));
                    if constexpr (sizeof(_Key_t) == 4)
                        _Unsigned_value ^= (_Unsigned_value & 0x80000000) ? 0xFFFFFFFF : 0x80000000;
                    else
                        _Unsigned_value ^= (_Unsigned_value & 0x8000000000000000) ? 0xFFFFFFFFFFFFFFFF : 0x8000000000000000;
                }
                else {
                    _Unsigned_value = std::bit_cast<_Unsigned_t>(_Extractor(_Start[_Idx]));
                }

                std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass * 8)) & _Mask;

                if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                if constexpr (std::is_signed_v<_Key_t> && !std::is_floating_point_v<_Key_t>) {
                    if (_Pass == _Passes - 1) _Byte_idx ^= _Bucket_size >> 1;
                }

                ++_Local_bucket[_Byte_idx];
            }
        }

        // Phase 2: Global reduction and prefix scan
        // Reduce local buckets to global buckets
        for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
            for (std::uint8_t _Thread = 0; _Thread < _Hardware_concurrency; ++_Thread) {
                _Global_bucket_count[_Bucket] += _Local_buckets[_Thread * _Bucket_size + _Bucket];
            }
        }

        // Compute global prefix sums
        std::array<std::size_t, _Bucket_size> _Global_prefix{};
        _Global_prefix[0] = 0;
        for (std::uint32_t _Bucket = 1; _Bucket < _Bucket_size; ++_Bucket) {
            _Global_prefix[_Bucket] = _Global_prefix[_Bucket - 1] + _Global_bucket_count[_Bucket - 1];
        }

        // Compute local offsets for each thread
        std::vector<std::size_t> _Local_offsets(_Bucket_size * _Hardware_concurrency);
        for (std::uint32_t _Bucket = 0; _Bucket < _Bucket_size; ++_Bucket) {
            std::size_t _Current_offset = _Global_prefix[_Bucket];
            for (std::uint8_t _Thread = 0; _Thread < _Hardware_concurrency; ++_Thread) {
                _Local_offsets[_Thread * _Bucket_size + _Bucket] = _Current_offset;
                _Current_offset += _Local_buckets[_Thread * _Bucket_size + _Bucket];
            }
        }

        // Phase 3: Parallel scattering
#pragma omp parallel num_threads(_Hardware_concurrency)
        {
            const int _Thread_id = omp_get_thread_num();
            std::size_t* _Local_offset = _Local_offsets.data() + _Thread_id * _Bucket_size;

#pragma omp for schedule(static)
            for (std::int64_t _Idx = 0; _Idx < _Size; ++_Idx) {
                auto _Value = _Start[_Idx];
                _Unsigned_t _Unsigned_value;

                if constexpr (std::is_floating_point_v<_Key_t>) {
                    _Unsigned_value = std::bit_cast<_Unsigned_t>(_Extractor(_Value));
                    if constexpr (sizeof(_Key_t) == 4)
                        _Unsigned_value ^= (_Unsigned_value & 0x80000000) ? 0xFFFFFFFF : 0x80000000;
                    else
                        _Unsigned_value ^= (_Unsigned_value & 0x8000000000000000) ? 0xFFFFFFFFFFFFFFFF : 0x8000000000000000;
                }
                else {
                    _Unsigned_value = std::bit_cast<_Unsigned_t>(_Extractor(_Value));
                }

                std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass * 8)) & _Mask;

                if constexpr (_Is_Descending) _Byte_idx = _Mask - _Byte_idx;

                if constexpr (std::is_signed_v<_Key_t> && !std::is_floating_point_v<_Key_t>) {
                    if (_Pass == _Passes - 1) _Byte_idx ^= _Bucket_size >> 1;
                }

                _End[_Local_offset[_Byte_idx]++] = _Value;
            }
        }

        std::swap(_Start, _End);
    }

}

/// <summary>
/// 版本2最终优化版特点：
/// 1. 优化的内存布局：转置存储结构，提高缓存局部性
/// 2. 减少内存分配：合并相关数组，减少动态内存分配
/// 3. 向量化友好：优化循环结构，便于编译器向量化
/// 4. 原地计算：避免不必要的中间数组
/// 5. 数据局部性优化：优化数据访问模式
/// </summary>
template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires ((_Bucket_size == 256U || _Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_impl_v2_final(std::execution::parallel_policy, ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor)
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
