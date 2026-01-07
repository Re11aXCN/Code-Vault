// 本测试测的是最后一趟分开，且V1、V2都进行 分块 #pragma omp parallel num_threads(Actual_threads)
// 仅在前缀和的数组存储方式和前缀和的计算略有不同
// 
// V1是桶计数和偏移量数组存储方式，V2是桶计数数组
// V1一个vector一个array，V2一个vector和两个array，V1的vector大小是V2vector大小两倍
// 
// V1版本性能更稳定，V2版本波动较大，虽然较少部分场景V2稍微优于V1，综合V1胜出
// 测试相同数据，不同数据看bechmark_radixsort_par2.hpp，也是V1胜出

#include "header.h"

#include <benchmark/benchmark.h>
#include "../stdex/profiling/generator.hpp"

#include <omp.h>
template<typename T>
struct identity_key_extractor {
    template<typename U>
    constexpr auto operator()(U&& value) const noexcept
        -> std::enable_if_t<std::is_same_v<std::decay_t<U>, T>, T>
    {
        return std::forward<U>(value);
    }
};

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t Bucket_size>
    requires ((Bucket_size == 256U || Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v1(std::execution::parallel_policy, ContigIter First, ContigIter Last, KeyExtractor&& Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)."
        "And not support reverse_iterator as range parameter.");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool> && sizeof(Key_t) >= 4,
        "Key type must be an arithmetic type (not bool) and at least 4 bytes. If you want to sort 8bits or 16bits, please using sequential_execution_policy.");

    using Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<Key_t>,
        std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>,
        Key_t>
    >;

    Value_t* Ptr = &*First;
    std::size_t Size = std::distance(First, Last);
    if (Size <= 1) return;

    constexpr std::uint8_t Passes = Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
    constexpr std::uint8_t Shift = Bucket_size == 256U ? 3 : 4;
    constexpr std::uint16_t Mask = Bucket_size - 1;

    constexpr std::size_t Key_bits = sizeof(Key_t) << 3;
    constexpr Unsigned_t Sign_bit_mask = Unsigned_t{ 1 } << (Key_bits - 1);
    constexpr Unsigned_t All_bits_mask = ~Unsigned_t{ 0 };

    constexpr bool Is_Descending = std::is_same_v<Compare, std::greater<>>;

    const std::int32_t Hardware_concurrency = omp_get_num_procs();

    // 优化：动态调整线程数，避免过小的数据块
    const std::size_t Min_chunk_size = 1024;
    const std::int32_t Actual_threads = std::max(1,
        std::min(Hardware_concurrency, static_cast<std::int32_t>(Size / Min_chunk_size)));

    const std::size_t Chunk = (Size + Actual_threads - 1) / Actual_threads;

    // 优化：合并本地桶计数和偏移量数组，减少内存分配
    // 布局: [thread0_bucket0, thread0_offset0, thread1_bucket0, thread1_offset0, ...]
    std::vector<std::size_t> Local_data(Actual_threads * Bucket_size * 2);

    // 辅助函数：获取线程的桶计数指针
    auto Func_get_local_buckets = [&](int Thread_id) -> std::size_t* {
        return Local_data.data() + Thread_id * Bucket_size * 2;
        };

    // 辅助函数：获取线程的偏移量指针
    auto Func_get_local_offsets = [&](int Thread_id) -> std::size_t* {
        return Local_data.data() + Thread_id * Bucket_size * 2 + Bucket_size;
        };

    // 优化：使用固定大小的全局前缀数组，避免动态分配
    std::array<std::size_t, Bucket_size> Global_prefix;

    std::vector<Value_t> Buffer(Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict Start = Ptr;
    Value_t* __restrict End = Buffer.data();
#else
    Value_t* Start = Ptr;
    Value_t* End = Buffer.data();
#endif
    for (std::uint8_t Pass = 0; Pass < Passes - 1; ++Pass) {
        {
            std::fill(Local_data.begin(), Local_data.end(), 0);

#pragma omp parallel num_threads(Actual_threads)
            {
                const int Thread_id = omp_get_thread_num();
                std::size_t* Local_buckets = Func_get_local_buckets(Thread_id);

                const std::size_t Start_idx = Thread_id * Chunk;
                const std::size_t End_idx = std::min(Size, (Thread_id + 1) * Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::size_t Idx = Start_idx; Idx < End_idx; ++Idx) {
                    Unsigned_t Unsigned_value = std::bit_cast<Unsigned_t>(Extractor(Start[Idx]));

                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if (Unsigned_value >> (Key_bits - 1)) Unsigned_value ^= All_bits_mask;
                    }

                    std::uint16_t Byte_idx = (Unsigned_value >> (Pass << Shift)) & Mask;

                    if constexpr (Is_Descending) Byte_idx = Mask - Byte_idx;

                    ++Local_buckets[Byte_idx];
                }
            }
        }

        {
            std::fill(Global_prefix.begin(), Global_prefix.end(), 0);

            for (std::int32_t Thread = 0; Thread < Actual_threads; ++Thread) {
                std::size_t* Local_buckets = Func_get_local_buckets(Thread);
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::uint32_t Bucket = 0; Bucket < Bucket_size; ++Bucket) {
                    Global_prefix[Bucket] += Local_buckets[Bucket];
                }
            }

            std::exclusive_scan(Global_prefix.begin(), Global_prefix.end(), Global_prefix.begin(), 0, std::plus<>{});

            for (std::int32_t Thread = 0; Thread < Actual_threads; ++Thread) {
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
                std::size_t* __restrict Local_buckets = Func_get_local_buckets(Thread);
                std::size_t* __restrict Local_offsets = Func_get_local_offsets(Thread);
#else
                std::size_t* Local_buckets = Func_get_local_buckets(Thread);
                std::size_t* Local_offsets = Func_get_local_offsets(Thread);
#endif
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::uint32_t Bucket = 0; Bucket < Bucket_size; ++Bucket) {
                    Local_offsets[Bucket] = Global_prefix[Bucket];
                    Global_prefix[Bucket] += Local_buckets[Bucket];
                }
            }
        }

        {
#pragma omp parallel num_threads(Actual_threads)
            {
                const int Thread_id = omp_get_thread_num();
                std::size_t* Local_offsets = Func_get_local_offsets(Thread_id);

                const std::size_t Start_idx = Thread_id * Chunk;
                const std::size_t End_idx = std::min(Size, (Thread_id + 1) * Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::size_t Idx = Start_idx; Idx < End_idx; ++Idx) {
                    auto Value = std::move(Start[Idx]);
                    Unsigned_t Unsigned_value = std::bit_cast<Unsigned_t>(Extractor(Value));

                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if (Unsigned_value >> (Key_bits - 1)) Unsigned_value ^= All_bits_mask;
                    }

                    std::uint16_t Byte_idx = (Unsigned_value >> (Pass << Shift)) & Mask;
                    if constexpr (Is_Descending) Byte_idx = Mask - Byte_idx;
                    End[Local_offsets[Byte_idx]++] = std::move(Value);
                }
            }
        }
        std::swap(Start, End);
    }

    // Signed Last Pass
    // ^^^^^^
    // Phase 1: 并行计数阶段
    {
        // 重置本地计数
        std::fill(Local_data.begin(), Local_data.end(), 0);

#pragma omp parallel num_threads(Actual_threads)
        {
            const int Thread_id = omp_get_thread_num();
            std::size_t* Local_buckets = Func_get_local_buckets(Thread_id);

            // 计算当前线程处理的区间
            const std::size_t Start_idx = Thread_id * Chunk;
            const std::size_t End_idx = std::min(Size, (Thread_id + 1) * Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            // 本地计数
            for (std::size_t Idx = Start_idx; Idx < End_idx; ++Idx) {
                Unsigned_t Unsigned_value = std::bit_cast<Unsigned_t>(Extractor(Start[Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        Unsigned_value ^= ((Unsigned_value >> (Key_bits - 1)) == 0) ? Sign_bit_mask : All_bits_mask;
                    }
                    else {
                        Unsigned_value ^= Sign_bit_mask;
                    }
                }

                std::uint16_t Byte_idx = (Unsigned_value >> ((Passes - 1) << Shift)) & Mask;

                if constexpr (Is_Descending) Byte_idx = Mask - Byte_idx;

                ++Local_buckets[Byte_idx];
            }
        }
    }

    // Phase 2: 优化的前缀和计算
    {
        // 步骤2.1: 全局归约 - 优化内存访问模式
        std::fill(Global_prefix.begin(), Global_prefix.end(), 0);

        for (std::int32_t Thread = 0; Thread < Actual_threads; ++Thread) {
            std::size_t* Local_buckets = Func_get_local_buckets(Thread);
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::uint32_t Bucket = 0; Bucket < Bucket_size; ++Bucket) {
                Global_prefix[Bucket] += Local_buckets[Bucket];
            }
        }

        // 步骤2.2: 计算全局前缀和
#if __cplusplus < 201703L
        std::size_t Running_sum = 0;
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
        for (std::uint32_t Bucket = 0; Bucket < Bucket_size; ++Bucket) {
            std::size_t Current_count = Global_prefix[Bucket];
            Global_prefix[Bucket] = Running_sum;
            Running_sum += Current_count;
        }
#else
        std::exclusive_scan(Global_prefix.begin(), Global_prefix.end(), Global_prefix.begin(), 0, std::plus<>{});
#endif
        // 步骤2.3: 计算本地偏移量 - 单次遍历完成
        for (std::int32_t Thread = 0; Thread < Actual_threads; ++Thread) {
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
            std::size_t* __restrict Local_buckets = Func_get_local_buckets(Thread);
            std::size_t* __restrict Local_offsets = Func_get_local_offsets(Thread);
#else
            std::size_t* Local_buckets = Func_get_local_buckets(Thread);
            std::size_t* Local_offsets = Func_get_local_offsets(Thread);
#endif
#if defined(__clang__)
#pragma clang loop vectorize(enable) interleave(enable) unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::uint32_t Bucket = 0; Bucket < Bucket_size; ++Bucket) {
                Local_offsets[Bucket] = Global_prefix[Bucket];
                Global_prefix[Bucket] += Local_buckets[Bucket];
            }
        }
    }

    // Phase 3: 并行散射阶段
    {
#pragma omp parallel num_threads(Actual_threads)
        {
            const int Thread_id = omp_get_thread_num();
            std::size_t* Local_offsets = Func_get_local_offsets(Thread_id);

            // 计算当前线程处理的区间
            const std::size_t Start_idx = Thread_id * Chunk;
            const std::size_t End_idx = std::min(Size, (Thread_id + 1) * Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            // 散射元素到正确位置
            for (std::size_t Idx = Start_idx; Idx < End_idx; ++Idx) {
                auto Value = std::move(Start[Idx]);
                Unsigned_t Unsigned_value = std::bit_cast<Unsigned_t>(Extractor(Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        Unsigned_value ^= ((Unsigned_value >> (Key_bits - 1)) == 0) ? Sign_bit_mask : All_bits_mask;
                    }
                    else {
                        Unsigned_value ^= Sign_bit_mask;
                    }
                }

                std::uint16_t Byte_idx = (Unsigned_value >> ((Passes - 1) << Shift)) & Mask;

                if constexpr (Is_Descending) Byte_idx = Mask - Byte_idx;

                End[Local_offsets[Byte_idx]++] = std::move(Value);
            }
        }
    }

    std::swap(Start, End);

    if constexpr (Passes & 1) std::move(Start, Start + Size, Ptr);
}

template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t Bucket_size>
    requires ((Bucket_size == 256U || Bucket_size == 65536U) &&
(std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
void radix_sort_v2(std::execution::parallel_policy, ContigIter First, ContigIter Last, KeyExtractor&& Extractor)
{
    static_assert(std::contiguous_iterator<ContigIter>,
        "Radix sort requires contiguous iterators (vector, array, raw pointers)."
        "And not support reverse_iterator as range parameter.");

    using Value_t = typename std::iterator_traits<ContigIter>::value_type;

    using Key_t = std::decay_t<decltype(Extractor(std::declval<Value_t>()))>;
    static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool> && sizeof(Key_t) >= 4,
        "Key type must be an arithmetic type (not bool) and at least 4 bytes. If you want to sort 8bits or 16bits, please using sequential_execution_policy.");

    using Unsigned_t = std::make_unsigned_t<
        std::conditional_t<std::is_floating_point_v<Key_t>,
        std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>,
        Key_t>
    >;

    Value_t* Ptr = &*First;
    std::size_t Size = std::distance(First, Last);
    if (Size <= 1) return;

    constexpr std::uint8_t Passes = Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
    constexpr std::uint8_t Shift = Bucket_size == 256U ? 3 : 4;
    constexpr std::uint16_t Mask = Bucket_size - 1;

    constexpr std::size_t Key_bits = sizeof(Key_t) << 3;
    constexpr Unsigned_t Sign_bit_mask = Unsigned_t{ 1 } << (Key_bits - 1);
    constexpr Unsigned_t All_bits_mask = ~Unsigned_t{ 0 };

    constexpr bool Is_Descending = std::is_same_v<Compare, std::greater<>>;

    // 动态调整线程数，避免过小的数据块
    const std::int32_t Hardware_concurrency = omp_get_num_procs();
    const std::size_t Min_chunk_size = 1024;
    const std::int32_t Actual_threads = std::max(1,
        std::min(Hardware_concurrency, static_cast<std::int32_t>(Size / Min_chunk_size)));
    const std::size_t Chunk = (Size + Actual_threads - 1) / Actual_threads;

    // 每个线程的本地桶计数数组
    std::vector<std::size_t> Processor_local_buckets(Bucket_size * Actual_threads);

    // 全局桶计数和前缀和数组
    std::array<std::size_t, Bucket_size> Bucket_count{};
    std::array<std::size_t, Bucket_size> Bucket_prefix{};

    std::vector<Value_t> Buffer(Size);
#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
    Value_t* __restrict Start = Ptr;
    Value_t* __restrict End = Buffer.data();
#else
    Value_t* Start = _Ptr;
    Value_t* End = _Buffer.data();
#endif

    for (std::uint8_t Pass = 0; Pass < Passes - 1; ++Pass) {
        // Phase 1: 并行计数阶段
        {
#pragma omp parallel num_threads(Actual_threads)
            {
                const int Thread_id = omp_get_thread_num();
                std::size_t* Plb_ptr = Processor_local_buckets.data() + Bucket_size * Thread_id;

                // 初始化本地桶计数
                std::fill(Plb_ptr, Plb_ptr + Bucket_size, 0);

                // 计算当前线程处理的区间
                const std::size_t Start_idx = Thread_id * Chunk;
                const std::size_t End_idx = std::min(Size, (Thread_id + 1) * Chunk);

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::size_t Idx = Start_idx; Idx < End_idx; ++Idx) {
                    Unsigned_t Unsigned_value = std::bit_cast<Unsigned_t>(Extractor(Start[Idx]));

                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if (Unsigned_value >> (Key_bits - 1)) Unsigned_value ^= All_bits_mask;
                    }

                    std::uint16_t Byte_idx = (Unsigned_value >> (Pass << Shift)) & Mask;

                    if constexpr (Is_Descending) Byte_idx = Mask - Byte_idx;

                    ++Plb_ptr[Byte_idx];
                }
            }
        }

        // Phase 2: 前缀和计算（串行）
        {
            // 重置Bucket_prefix数组
            Bucket_prefix.fill(0);

            // 外层循环遍历线程，内层循环遍历桶
            for (std::int32_t Core = 0; Core < Actual_threads; ++Core) {
                for (std::size_t Idx = 0; Idx < Bucket_size; ++Idx) {
                    std::size_t Idx_local = Bucket_size * Core + Idx;
                    std::size_t Temp_count_local = Processor_local_buckets[Idx_local];

                    // 将本地桶计数转换为偏移量
                    Processor_local_buckets[Idx_local] = Bucket_prefix[Idx];
                    Bucket_prefix[Idx] += Temp_count_local;
                }
            }

            // 计算全局桶计数
            std::exclusive_scan(Bucket_prefix.begin(), Bucket_prefix.end(), Bucket_count.begin(), 0);
        }

        // Phase 3: 并行散射阶段
        {
#pragma omp parallel num_threads(Actual_threads)
            {
                const int Thread_id = omp_get_thread_num();
                std::size_t* Plb_ptr = Processor_local_buckets.data() + Bucket_size * Thread_id;
                std::transform(Plb_ptr, Plb_ptr + Bucket_size, Bucket_count.data(), Plb_ptr, std::plus<>{});

                // 计算当前线程处理的区间
                const std::size_t Start_idx = Thread_id * Chunk;
                const std::size_t End_idx = std::min(Size, (Thread_id + 1) * Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
                for (std::size_t Idx = Start_idx; Idx < End_idx; ++Idx) {
                    auto Value = std::move(Start[Idx]);
                    Unsigned_t Unsigned_value = std::bit_cast<Unsigned_t>(Extractor(Value));

                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if (Unsigned_value >> (Key_bits - 1)) Unsigned_value ^= All_bits_mask;
                    }

                    std::uint16_t Byte_idx = (Unsigned_value >> (Pass << Shift)) & Mask;

                    if constexpr (Is_Descending) Byte_idx = Mask - Byte_idx;

                    End[Plb_ptr[Byte_idx]++] = std::move(Value);
                }
            }
        }

        std::swap(Start, End);
    }

    // Phase 1: 并行计数阶段
    {
#pragma omp parallel num_threads(Actual_threads)
        {
            const int Thread_id = omp_get_thread_num();
            std::size_t* Plb_ptr = Processor_local_buckets.data() + Bucket_size * Thread_id;

            // 初始化本地桶计数
            std::fill(Plb_ptr, Plb_ptr + Bucket_size, 0);

            // 计算当前线程处理的区间
            const std::size_t Start_idx = Thread_id * Chunk;
            const std::size_t End_idx = std::min(Size, (Thread_id + 1) * Chunk);

#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t Idx = Start_idx; Idx < End_idx; ++Idx) {
                Unsigned_t Unsigned_value = std::bit_cast<Unsigned_t>(Extractor(Start[Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        Unsigned_value ^= ((Unsigned_value >> (Key_bits - 1)) == 0) ? Sign_bit_mask : All_bits_mask;
                    }
                    else {
                        Unsigned_value ^= Sign_bit_mask;
                    }
                }

                std::uint16_t Byte_idx = (Unsigned_value >> ((Passes - 1) << Shift)) & Mask;

                if constexpr (Is_Descending) Byte_idx = Mask - Byte_idx;

                ++Plb_ptr[Byte_idx];
            }
        }
    }

    // Phase 2: 前缀和计算（串行）
    {
        Bucket_prefix.fill(0);

        for (std::int32_t Core = 0; Core < Actual_threads; ++Core) {
            for (std::size_t Idx = 0; Idx < Bucket_size; ++Idx) {
                std::size_t Idx_local = Bucket_size * Core + Idx;
                std::size_t Temp_count_local = Processor_local_buckets[Idx_local];

                Processor_local_buckets[Idx_local] = Bucket_prefix[Idx];
                Bucket_prefix[Idx] += Temp_count_local;
            }
        }

        std::exclusive_scan(Bucket_prefix.begin(), Bucket_prefix.end(), Bucket_count.begin(), 0);
    }

    // Phase 3: 并行散射阶段
    {
#pragma omp parallel num_threads(Actual_threads)
        {
            const int Thread_id = omp_get_thread_num();
            std::size_t* Plb_ptr = Processor_local_buckets.data() + Bucket_size * Thread_id;
            std::transform(Plb_ptr, Plb_ptr + Bucket_size, Bucket_count.data(), Plb_ptr, std::plus<>{});

            // 计算当前线程处理的区间
            const std::size_t Start_idx = Thread_id * Chunk;
            const std::size_t End_idx = std::min(Size, (Thread_id + 1) * Chunk);
#if defined(__clang__)
#pragma clang loop unroll_count(8)
#elif defined(__GNUC__) && !defined(__INTEL_COMPILER)
#pragma GCC unroll 8
#elif defined(_MSC_VER)
#endif
            for (std::size_t Idx = Start_idx; Idx < End_idx; ++Idx) {
                auto Value = std::move(Start[Idx]);
                Unsigned_t Unsigned_value = std::bit_cast<Unsigned_t>(Extractor(Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        Unsigned_value ^= ((Unsigned_value >> (Key_bits - 1)) == 0) ? Sign_bit_mask : All_bits_mask;
                    }
                    else {
                        Unsigned_value ^= Sign_bit_mask;
                    }
                }

                std::uint16_t Byte_idx = (Unsigned_value >> ((Passes - 1) << Shift)) & Mask;

                if constexpr (Is_Descending) Byte_idx = Mask - Byte_idx;

                End[Plb_ptr[Byte_idx]++] = std::move(Value);
            }
        }
    }

    std::swap(Start, End);
    if constexpr (Passes & 1) std::move(Start, Start + Size, Ptr);
}

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
template<typename T, typename SortFunction, bool Ascending = true>
void BM_Sort_Typed_Helper2(benchmark::State& state) {
    auto size = state.range(0);

    static bool initialized = []() {
        TestDataCache<T>::initialize();
        //initialize_all_test_data();
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

static void BM_Sort_int32_t_RadixSortV1Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<int32_t, ParallelRadixSortV1Wrapper, true>(state);
}

static void BM_Sort_int32_t_RadixSortV2Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<int32_t, ParallelRadixSortV2Wrapper, true>(state);
}

BENCHMARK(BM_Sort_int32_t_RadixSortV1Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK(BM_Sort_int32_t_RadixSortV2Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

static void BM_Sort_uint32_t_RadixSortV1Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<uint32_t, ParallelRadixSortV1Wrapper, true>(state);
}

static void BM_Sort_uint32_t_RadixSortV2Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<uint32_t, ParallelRadixSortV2Wrapper, true>(state);
}

BENCHMARK(BM_Sort_uint32_t_RadixSortV1Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK(BM_Sort_uint32_t_RadixSortV2Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

static void BM_Sort_int64_t_RadixSortV1Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<int64_t, ParallelRadixSortV1Wrapper, true>(state);
}

static void BM_Sort_int64_t_RadixSortV2Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<int64_t, ParallelRadixSortV2Wrapper, true>(state);
}

BENCHMARK(BM_Sort_int64_t_RadixSortV1Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK(BM_Sort_int64_t_RadixSortV2Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

static void BM_Sort_uint64_t_RadixSortV1Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<uint64_t, ParallelRadixSortV1Wrapper, true>(state);
}

static void BM_Sort_uint64_t_RadixSortV2Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<uint64_t, ParallelRadixSortV2Wrapper, true>(state);
}

BENCHMARK(BM_Sort_uint64_t_RadixSortV1Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK(BM_Sort_uint64_t_RadixSortV2Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

static void BM_Sort_float_RadixSortV1Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<float, ParallelRadixSortV1Wrapper, true>(state);
}

static void BM_Sort_float_RadixSortV2Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<float, ParallelRadixSortV2Wrapper, true>(state);
}

BENCHMARK(BM_Sort_float_RadixSortV1Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK(BM_Sort_float_RadixSortV2Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();


static void BM_Sort_double_RadixSortV1Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<double, ParallelRadixSortV1Wrapper, true>(state);
}

static void BM_Sort_double_RadixSortV2Wrapper_ascending(benchmark::State& state) {
    BM_Sort_Typed_Helper2<double, ParallelRadixSortV2Wrapper, true>(state);
}

BENCHMARK(BM_Sort_double_RadixSortV1Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK(BM_Sort_double_RadixSortV2Wrapper_ascending)
->Arg(1024)->Arg(4096)->Arg(16384)->Arg(65536)->Arg(262144)->Arg(1048576)
->Unit(benchmark::kMillisecond)->Complexity();

BENCHMARK_MAIN();