/*
 * @lc app=leetcode.cn id=15 lang=cpp
 *
 * [15] 三数之和
 */

// @lc code=start
#include <vector>
using std::vector;
class Solution {
public:
    vector<vector<int>> threeSum(vector<int>& nums) {
        // 桶 + 基数排序，适用于整数O(n)
        this->radix_sort(nums.data(), nums.size());
        if (nums.front() > 0 || nums.back() < 0) return {};

        vector<vector<int>> three_tuples;
        three_tuples.reserve(10);
        
        // 外层循环负数/0
        for(auto it = nums.begin(); it != nums.end() - 2; ++it) {
            // 外层负数跳过重复元素
            if(it != nums.begin() && *it == *(it - 1)) continue;
            
            // 循环到正数结束
            if(*it > 0) break;

            auto left{ std::next(it) };
            auto right{ std::prev(nums.end()) };

            // 优化特殊情况
            if ((*it + *left + *(left + 1)) > 0) [[unlikely]]  break; // 最后一个负数优化
            if ((*it + *right + *(right - 1)) < 0) continue;

            // 内层循环找一个负数和一个正数 / 两个正数 / 0
            while(left < right) {
                const int sum = *it + *left + *right;

                if(sum == 0) {
                    three_tuples.push_back({*it, *left, *right});

                    // 内层跳过重复的元素，注意和外层的区别
                    while (left < right && *left == *std::next(left)) ++left;
                    while (left < right && *right == *std::prev(right)) --right;

                    // 下一轮循环
                    ++left, --right;
                }
                else if (sum < 0) ++left;
                else /*if (sum > 0)*/ --right;
            }
        }
        return three_tuples;
    }
private:
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
};
// @lc code=end
/*
 * @lc app=leetcode.cn id=15 lang=cpp
 *
 * [15] 三数之和
 */

// @lc code=start
#include <set>
#include <execution>
#include <unordered_set>
#include <ranges>
#include <vector>
#include <string>
#include <optional>
#include <iterator>
#include <algorithm>
#include <concepts>
#include <iostream>

template <std::ranges::random_access_range R, typename T>
requires std::totally_ordered_with<T, std::ranges::range_value_t<R>>
inline std::optional<std::size_t> binary_search(const R& container, const T& value) {
    auto begin = std::ranges::begin(container);
    auto end = std::ranges::end(container);
    
    // 使用标准库 lower_bound 实现核心逻辑
    auto it = std::lower_bound(begin, end, value);
    
    // C++23 增强条件检查
    if (it != end && !(value < *it)) { // 等价于 *it == value
        return std::distance(begin, it);
    }
    return std::nullopt;
}
bool operator<(const vector<int>& a, const vector<int>& b) {
    return tie(a[0], a[1], a[2]) < tie(b[0], b[1], b[2]);
}

vector<vector<int>> findDifferences(const vector<vector<int>>& arr1, const vector<vector<int>>& arr2) {
    set<vector<int>> set1(arr1.begin(), arr1.end());
    set<vector<int>> set2(arr2.begin(), arr2.end());

    vector<vector<int>> uniqueToArr1, uniqueToArr2;

    // 找到在 arr1 中但不在 arr2 中的元素
    for (const auto& item : set1) {
        if (set2.find(item) == set2.end()) {
            uniqueToArr1.push_back(item);
        }
    }

    // 找到在 arr2 中但不在 arr1 中的元素
    for (const auto& item : set2) {
        if (set1.find(item) == set1.end()) {
            uniqueToArr2.push_back(item);
        }
    }

    // 合并结果
    uniqueToArr1.insert(uniqueToArr1.end(), uniqueToArr2.begin(), uniqueToArr2.end());
    return uniqueToArr1;
}
void Test(const vector<vector<int>>& arr2)
{
    vector<vector<int>> arr1 = {
   {-82, -11, 93}, {-82, 13, 69}, {-82, 17, 65}, {-82, 21, 61}, {-82, 26, 56},
   {-82, 33, 49}, {-82, 34, 48}, {-82, 36, 46}, {-70, -14, 84}, {-70, -6, 76},
   {-70, 1, 69}, {-70, 13, 57}, {-70, 15, 55}, {-70, 21, 49}, {-70, 34, 36},
   {-66, -11, 77}, {-66, -3, 69}, {-66, 1, 65}, {-66, 10, 56}, {-66, 17, 49},
   {-49, -6, 55}, {-49, -3, 52}, {-49, 1, 48}, {-49, 2, 47}, {-49, 13, 36},
   {-49, 15, 34}, {-49, 21, 28}, {-43, -14, 57}, {-43, -6, 49}, {-43, -3, 46},
   {-43, 10, 33}, {-43, 12, 31}, {-43, 15, 28}, {-43, 17, 26}, {-29, -14, 43},
   {-29, 1, 28}, {-29, 12, 17}, {-14, -3, 17}, {-14, 1, 13}, {-14, 2, 12},
   {-11, -6, 17}, {-11, 1, 10}, {-3, 1, 2}
    };

    vector<vector<int>> differences = findDifferences(arr1, arr2);

    cout << "不同的数组元素:\n";
    for (const auto& item : differences) {
        cout << "[" << item[0] << ", " << item[1] << ", " << item[2] << "]\n";
    }
}
using namespace std;
class Solution {
public:
    vector<vector<int>> threeSum(vector<int>& nums) {
        vector<vector<int>> three_sums;
#ifdef BRUTE_FORCE
#ifndef BRUTE_FORCE_OPTIZATION
        unordered_set<string> record;
        for(size_t i = 0; i < nums.size(); ++i)
        {
            for(size_t j = i + 1; j < nums.size(); ++j)
            {
                for(size_t k = j + 1; k < nums.size(); ++k)
                {
                    if(!(nums[i] + nums[j] + nums[k]))
                    {
                        array<int, 3> three_tuple{nums[i],nums[j],nums[k]}; 
                        ranges::sort(three_tuple);
                        string id;
                        id.reserve(3);
                        for (const auto& num : three_tuple) {
                            id += to_string(num);
                        }
                        if(!record.contains(id))
                        {
                            record.insert(id);
                            three_sums.push_back({nums[i],nums[j],nums[k]});
                        }
                    }
                }
            }
        }
#else
// 负数找0找两个正数，正数找0找两个负数
// -4 -1 -1 0 1 2
        ranges::sort(nums);
        if (nums[0] >= 0) {
            if (nums[0] == 0 && nums[1] == 0 && nums[2] == 0)
                return { {0,0,0} };
            else
                return {};
        }
        const size_t length = nums.size();
        auto begin_iter = nums.cbegin();
        auto end_iter = nums.cend();
        size_t nonnegative_index = std::distance(begin_iter, (length > 300 ? find_if(execution::par, begin_iter, end_iter, [](int n) { return n >= 0; })
            : find_if(execution::seq, begin_iter, end_iter, [](int n) { return n >= 0; })));
        unordered_set<int> filter_set;
        for (int i = 0; i < nonnegative_index; ++i)
        {
            if (nums[i] == nums[i+1]) continue;
            for (int j = nonnegative_index; j < length; ++j)
            {
                if (nums[i] + nums[j] > 0) break;
                if (j + 1 < length && nums[j] == nums[j + 1])
                {
                    if(nums[j] == nums[j - 1] || nums[j] == 0)
                        continue;
                }
                for (int k = j + 1; k < length; ++k)
                {
                    if (nums[i] + nums[k] > 0) break;
                    int id = -nums[i] * 100 + nums[j] * 10 + nums[k];
                    if (filter_set.contains(id)) continue;
                    if ((nums[i] + nums[j] + nums[k]) == 0)
                    {
                        filter_set.insert(-nums[i] * 100 + nums[j] * 10 + nums[k]);
                        three_sums.push_back({ nums[i],nums[j],nums[k] });
                    }
                }
            }
        }
        for (int i = length - 1; i >= nonnegative_index; --i)
        {
            if (nums[i] == nums[i-1]) continue;
            for (int j = nonnegative_index - 1; j >= 0; --j)
            {
                if (nums[i] + nums[j] < 0) break;
                if (j - 1 >= 0 && nums[j] == nums[j - 1])
                {
                    if (nums[j] == nums[j + 1] || nums[j] == 0)
                        continue;
                }
                for (int k = j - 1; k >= 0; --k)
                {
                    if (nums[i] + nums[k] < 0) break;
                    int id = -nums[k] * 100 + nums[j] * 10 + nums[i];
                    if (filter_set.contains(id)) continue;
                    if ((nums[i] + nums[j] + nums[k]) == 0)
                    {
                        filter_set.insert(-nums[k] * 100 + nums[j] * 10 + nums[i]);
                        three_sums.push_back({ nums[k],nums[j],nums[i] });
                    }
                }
            }
        }
        if (nums[nonnegative_index] == 0 && length >= nonnegative_index + 3)
        {
            if(nums[nonnegative_index + 1] == 0 && nums[nonnegative_index + 2] == 0)
                three_sums.push_back({ 0,0,0 });
        }
#endif BRUTE_FORCE_OPTIZATION
#else
        /// <describe>
        /// 双指针
        /// 给你一个整数数组 nums ，判断是否存在三元组 [nums[i], nums[j], nums[k]] 满足 i != j、i != k 且 j != k ，同时还满足 nums[i] + nums[j] + nums[k] == 0 。请你返回所有和为 0 且不重复的三元组。
        /// 注意：答案中不可以包含重复的三元组。
        /// </describe>
        /*
        * 思路：
        * 正（负）数，0，负（正）数； 正（负）数，负（正）数，负（正）数；0 0 0；可以相加=0
        * 不重复意味着唯一，即同一个数进行上述操作得到结构一样，我们需要跳过
        * 可以看到两种情况都有一个负数，找三个数，那么我们可以这样思考，外层循环负数和0（0比较特殊），固定i
        * 如果值大于0，结束循环
        * 内层循环找另外两个数j和k，通过双指针，循环条件left小于right
        * left向右，right向左，left初始是负数，right初始是正数，移动条件是计算i和left和right的和
        * 如果和小于0，说明left对应的元素小了，需要向右移动
        * 如果和大于0，说明right对应元素大了，需要向左移动
        * 如果和等于0，说明找到了，中间可能存在重复元素，循环移动left和后一个比较直到不相等，且left<right；注意结束后还要++才是不相等元素
               循环移动right和前一个比较直到不相等，且left<right；注意结束后还要--才是不相等元素
        * 上述思路是建立在nums有序的条件下
        */
        const int length = nums.size();
        ranges::sort(nums);
        for (int i = 0; i < length-2; ++i)
        {
            // 跳过重复元素
            if (i > 0 && nums[i] == nums[i-1]) continue;
            // 提前终止条件
            if (nums[i] > 0) break;
              
            int left = i + 1;
            int right = length - 1;
            while (left < right) {
                const int sum = nums[i] + nums[left] + nums[right];
                
                if (sum == 0) {
                    three_sums.push_back({nums[i], nums[left], nums[right]});
                    
                    // 跳过重复元素
                    while (left < right && nums[left] == nums[left+1]) ++left;
                    while (left < right && nums[right] == nums[right-1]) --right;
                    
                    ++left;
                    --right;
                } else if (sum < 0) {
                    ++left;
                } else {
                    --right;
            }
        }
    }

#endif BRUTE_FORCE
        return three_sums;
    }
};
// @lc code=end
#ifdef _DEBUG

int main()
{
    
    Solution obj;
    vector<int> nums{ -1,0,0,1 };
    Test(obj.threeSum(nums));
    return 0;
}
#endif

