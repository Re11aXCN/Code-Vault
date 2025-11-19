// 最优写法 3ms
class Solution {
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
public:
    int longestConsecutive(std::vector<int>& nums) {
        if(nums.empty()) return 0;

        radix_sort(nums.data(), nums.size());
        int maxLen = 1, currentLen = 1;
        int n = nums.size();
        
        #pragma GCC unroll 4
        for (int i = 1; i < n; i++) {
            if (nums[i] == nums[i-1] + 1) {
                currentLen++;
                maxLen = maxLen > currentLen ? maxLen : currentLen;
            } else if (nums[i] != nums[i-1]) {
                currentLen = 1;
            }
        }
        
        return maxLen;
    }
};


// 比std::sort慢
int longestConsecutive(vector<int>& nums) {
    if (nums.empty()) return 0;
    
    // 使用unordered_set去重，查找效率O(1)
    unordered_set<int> numSet(nums.begin(), nums.end());
    int longest = 0;
    
    for (int num : numSet) {
        // 只有当num是序列的起点时才进入循环
        // 即num-1不在set中，说明这是新序列的开始
        if (numSet.find(num - 1) == numSet.end()) {
            int currentNum = num;
            int currentStreak = 1;
            
            // 向后查找连续的数字
            while (numSet.find(currentNum + 1) != numSet.end()) {
                currentNum++;
                currentStreak++;
            }
            
            // 更新最长连续序列长度
            if(longest < currentStreak) longest = currentStreak;
        }
    }
    
    return longest;
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
        if (nums.empty()) return 0;
        int max_length = 1;
#define SORT
#ifdef SORT
        ranges::sort(nums);
        int current_len = 1; // 当前连续长度

        // 单次遍历检查相邻元素
        for (size_t i = 1; i < nums.size(); ++i) {
            if (nums[i] == nums[i - 1] + 1) { // 严格连续递增
                ++current_len;
                max_length = std::max(max_length, current_len);
            }
            else if (nums[i] != nums[i - 1]) { // 非重复元素时重置
                current_len = 1;
            }
            // 若元素重复则跳过，保持当前长度不变
        }
#else
        unordered_set<int> num_set(nums.begin(), nums.end());
        // C++23范围视图：筛选没有前驱的数字作为序列起点
        auto start_points = num_set | views::filter([&](int num) {
#ifdef _DEBUG

            cout << format("filter num is {}", num) << endl;
#endif
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

            max_length = max(max_length, length);
        }
#endif // SORT
        return max_length;
    }
};
// @lc code=end
#ifdef _DEBUG
int main()
{
    Solution obj;
    vector<int> nums{ 0,3,7,2,5,8,4,6,0,1,12,15,16 };
    obj.longestConsecutive(nums);
    return 0;
}
#endif

