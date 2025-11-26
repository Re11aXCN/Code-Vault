// 标准写法
// 2O(n)，建表，循环，空间O(n)
int longestConsecutive(vector<int>& nums) {
    std::unordered_set<int> num_set(nums.begin(), nums.end());
    int max_length = 0;
    for (int num : num_set) {
        if (num_set.find(num - 1) == num_set.end()) {
            int current = num;
            int length = 1;
            while (num_set.find(++current) != num_set.end()) {
                ++length;
            }
            if(max_length < length) max_length = length;
            if(max_length * 2 >= nums.size()) break;
        }
    }

    return max_length;
}
/*
 * @lc app=leetcode.cn id=128 lang=cpp
 *
 * [128] 最长连续序列
 */

// @lc code=start
#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

#include <vector>
#include <ranges>
#include <algorithm>
#include <unordered_set>
using namespace std;
class Solution {
public:
    //方案一：计数排序，然后再遍历找最长length，
    //  2O（n）复杂度，但是num[i] <= 10^9 不太现实
    //方案二：普通排序，然后再遍历找最长length，
    //  时间复杂度是排序时间+O(n)
    //方案三：过滤出无重复的没有前驱的数字集合，然后遍历这个集合统计长度，
    //  时间复杂度低于O(n^2)，但是大于2*O(n)，实际是求和n次找到前驱的时间

    //实际情况考虑到量级[0, 10^5]次方，最优的情况应该采用方案二
     /*  步骤
      * 1、使用标准库的排序，排序数组
      * 2、循环查找严格连续递增的序列，使用临时长度变量，max统计其最大长度，
           如果相差不等于1，则重置临时长度为1，重复元素跳过
      */
    int longestConsecutive(vector<int>& nums) {
#define SORT
#ifdef SORT
        if (nums.empty()) return 0;
        std::sort(nums.begin(), nums.end());
        int currLen = 1, maxLen = currLen;
        #pragma clang loop unroll_count(8)
        for (auto it = std::next(nums.begin()); it != nums.end(); ++it) {
            int prev_val = *std::prev(it);
            if (prev_val == *it) continue;
            if (prev_val + 1 == *it) {
                if (++currLen > maxLen) maxLen = currLen;
                if (maxLen * 2 >= nums.size()) break;
            }
            else {
                currLen = 1;
            }
        }
        return maxLen;
#else
        int max_length = 1;
        std::unordered_set<int> num_set(nums.begin(), nums.end()); // 替换为boost::unordered::unordered_flat_set（开放地址） 性能提升4~7倍
        int max_length = 0;
        // C++23范围视图：筛选没有前驱的数字作为序列起点
        auto start_points = num_set | std::views::filter([&](int num) {
            return !num_set.contains(num - 1);
            });
        // 遍历所有起点，扩展连续序列
        for (int start : start_points) {
            int current = start;
            int length = 1;
            // 向后扩展序列直到不连续
            while (num_set.contains(++current)) {
                ++length;
            }

            if(max_length < length) max_length = length;
            if(max_length * 2 >= nums.size()) break;
        }
        return max_length;
#endif // SORT
    }
};
// @lc code=end

// 最快基数排序，符合O(n), 排序8n + 8(256), 查找O(n), 空间 O(n) + 2 * 256
class Solution {
public: 
    int longestConsecutive(vector<int>& nums) {
        if (nums.empty()) return 0;
        
        using Iter_t = decltype(nums.begin());
        using Value_t = typename std::iterator_traits<Iter_t>::value_type;  
        radixsort<Iter_t, std::less<>, identity_key_extractor<Value_t>, 256U>(nums.begin(), nums.end(), {});

        int currLen = 1, maxLen = currLen;
        #pragma clang loop unroll_count(8)
        for (auto it = std::next(nums.begin()); it != nums.end(); ++it) {
            int prev_val = *std::prev(it);
            if (prev_val == *it) continue;
            if (prev_val + 1 == *it) {
                if (++currLen > maxLen) maxLen = currLen;
                if (maxLen * 2 >= nums.size()) break;
            }
            else {
                currLen = 1;
            }
        }
        return maxLen;
    }

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
    requires((_Bucket_size == 256U || _Bucket_size == 65536U) && 
             (std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>))
    void radixsort(ContigIter _First, ContigIter _Last, KeyExtractor _Extractor)
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

        Value_t* __restrict _Start = _Ptr;
        Value_t* __restrict _End = _Buffer.data();

        for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
            std::fill(_Bucket_count.begin(), _Bucket_count.end(), 0);
  
            for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Start[_Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value < 0) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value < 0) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

            std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(), _Scanned.begin(), 0, std::plus<>{});
            
            for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
                auto _Value = _Start[_Idx];
                Unsigned_t _Unsigned_value = std::bit_cast<Unsigned_t>(_Extractor(_Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value < 0) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else
                            _Unsigned_value ^= (_Unsigned_value < 0) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
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

                _End[_Scanned[_Byte_idx]++] = _Value;
            }
            
            std::swap(_Start, _End);
        }
        
        if constexpr (sizeof(Key_t) == 1) std::copy_n(_Start, _Size, _Ptr);
    }
};

