#include <type_traits>
#include <vector>
#include <array>
#include <algorithm>
#include <bit>
#include <iostream>
#include <print>
#include <string_view>
#include <chrono>
#include <random>
#include <omp.h>

template<typename _NumericType, std::uint32_t _Bucket_size = 256U, typename _MaxCapacityType = uint32_t>
    requires (std::is_arithmetic_v<_NumericType> && !std::is_same_v<_NumericType, bool> && (_Bucket_size == 256U || _Bucket_size == 65536U))
void radix_sort(_NumericType* _Ptr, std::size_t _Size)
{
#pragma push_macro("min")
#undef min
    static_assert(
        std::is_same_v<_MaxCapacityType, uint32_t> ||
        std::is_same_v<_MaxCapacityType, size_t> ||
        std::is_same_v<_MaxCapacityType, uint64_t> ||
        std::is_same_v<_MaxCapacityType, uintmax_t> ||
        std::is_same_v<_MaxCapacityType, unsigned> ||
        std::is_same_v<_MaxCapacityType, unsigned int> ||
        std::is_same_v<_MaxCapacityType, unsigned long long>,
        "T must be an unsigned integer type (uint32_t, size_t, uint64_t, etc.)"
        );
    using _Unsigned = std::make_unsigned_t<_NumericType>;

    if (_Size <= 1) return;

    constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(_NumericType) : (sizeof(_NumericType) + 1) >> 1;
    constexpr std::uint16_t _Mask = _Bucket_size - 1; // 0xFF for 8-bit, 0xFFFF for 16-bit, etc.

    std::array<_MaxCapacityType, _Bucket_size> _Bucket_count{};
#ifdef USE_RADIX_SORT_OMP_PARALLEL
    std::uint8_t _Hardware_concurrency{ (std::uint8_t)(omp_get_num_procs()) };
    std::size_t _Chunk = (_Size + _Hardware_concurrency - 1) / _Hardware_concurrency;
    std::vector<_MaxCapacityType> _Processor_local_buckets(_Bucket_size * _Hardware_concurrency);
#else
    std::array<_MaxCapacityType, _Bucket_size> _Scanned{};
#endif // USE_RADIX_SORT_OMP_PARALLEL

    std::vector<_NumericType> _Buffer(_Size);
    _NumericType* _Start = _Ptr;
    _NumericType* _End = _Buffer.data();

    for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
#ifdef USE_RADIX_SORT_OMP_PARALLEL
#pragma omp parallel for schedule(static, 1)
        for (std::uint8_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            _MaxCapacityType* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            for (std::size_t _Idx = 0; _Idx < _Bucket_size; ++_Idx)  _Plb_ptr[_Idx] = 0;

            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                if constexpr (std::is_signed_v<_NumericType>) {
                    _Unsigned _Value = std::bit_cast<_Unsigned>(_Start[_Idx]);
                    std::uint16_t _Byte_idx = (_Value >> (_Pass * 8)) & _Mask;

                    if (_Pass == _Passes - 1) _Byte_idx ^= _Bucket_size >> 1;

                    ++_Plb_ptr[_Byte_idx];
                }
                else {
                    ++_Plb_ptr[(_Start[_Idx] >> (_Pass * 8)) & _Mask];
                }
            }
        }
#else
        std::fill(_Bucket_count.begin(), _Bucket_count.end(), 0);
        // Count the number of elements per bucket
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            if constexpr (std::is_signed_v<_NumericType>) {
                _Unsigned _Value = std::bit_cast<_Unsigned>(_Start[_Idx]);
                std::uint16_t _Byte_idx = (_Value >> (_Pass * 8)) & _Mask;

                if (_Pass == _Passes - 1) _Byte_idx ^= _Bucket_size >> 1;

                ++_Bucket_count[_Byte_idx];
            }
            else {
                ++_Bucket_count[(_Start[_Idx] >> (_Pass * 8)) & _Mask];
            }
        }
#endif // USE_RADIX_SORT_OMP_PARALLEL
        // Calculate the sum of prefixes by exclusive scan
#ifdef USE_RADIX_SORT_OMP_PARALLEL
        std::size_t _Temp_sum{ 0 };
        for (std::size_t _Idx = 0; _Idx < _Bucket_size; ++_Idx) {
            std::size_t _Temp_sum_local{ 0 };
            for (std::uint8_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
                std::size_t _Idx_local = _Bucket_size * _Core + _Idx;
                _MaxCapacityType _Temp_count_local = _Processor_local_buckets[_Idx_local];
                _Processor_local_buckets[_Idx_local] = _Temp_sum_local;
                _Temp_sum_local += _Temp_count_local;
            }
            _Bucket_count[_Idx] = _Temp_sum;
            _Temp_sum += _Temp_sum_local;
        }
#else
        _Scanned[0] = 0;
        for (std::size_t _Idx = 1; _Idx < _Bucket_size; ++_Idx) {
            _Scanned[_Idx] = _Scanned[_Idx - 1] + _Bucket_count[_Idx - 1];
        }
#endif // USE_RADIX_SORT_OMP_PARALLEL

#ifdef USE_RADIX_SORT_OMP_PARALLEL
#pragma omp parallel for schedule(static, 1)
        for (std::uint8_t _Core = 0; _Core < _Hardware_concurrency; ++_Core) {
            _MaxCapacityType* _Plb_ptr = _Processor_local_buckets.data() + _Bucket_size * _Core;
            for (std::size_t _Idx = 0; _Idx < _Bucket_size; ++_Idx)  _Plb_ptr[_Idx] += _Bucket_count[_Idx];

            for (std::size_t _Idx = _Core * _Chunk, _EIdx = std::min(_Size, (_Core + 1) * _Chunk); _Idx < _EIdx; ++_Idx) {
                if constexpr (std::is_signed_v<_NumericType>) {
                    _Unsigned _Value = std::bit_cast<_Unsigned>(_Start[_Idx]);
                    std::uint16_t _Byte_idx = (_Value >> (_Pass * 8)) & _Mask;

                    if (_Pass == _Passes - 1) _Byte_idx ^= _Bucket_size >> 1;

                    _End[_Plb_ptr[_Byte_idx]++] = _Start[_Idx];
                }
                else {
                    _End[_Plb_ptr[(_Start[_Idx] >> (_Pass * 8)) & _Mask]++] = _Start[_Idx];
                }
            }
        }
#else
        // Move elements to their final positions
        for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
            if constexpr (std::is_signed_v<_NumericType>) {
                _Unsigned _Value = std::bit_cast<_Unsigned>(_Start[_Idx]);
                std::uint16_t _Byte_idx = (_Value >> (_Pass * 8)) & _Mask;

                if (_Pass == _Passes - 1) _Byte_idx ^= _Bucket_size >> 1;

                _End[_Scanned[_Byte_idx]++] = _Start[_Idx];
            }
            else {
                std::uint16_t _Byte_idx = (_Start[_Idx] >> (_Pass * 8)) & _Mask;
                _End[_Scanned[_Byte_idx]++] = _Start[_Idx];
            }

        }
#endif // USE_RADIX_SORT_OMP_PARALLEL
        // Swap buffer
        std::swap(_Start, _End);
    }

    if (_Start != _Ptr)  std::copy_n(_Start, _Size, _Ptr);
#pragma pop_macro("min")
}