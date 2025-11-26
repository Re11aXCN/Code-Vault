    int findKthLargest(vector<int>& nums, int k) {
        using Iter_t = decltype(nums.begin());
        using Value_t = typename std::iterator_traits<Iter_t>::value_type;
        radixsort<Iter_t, std::less<>, identity_key_extractor<Value_t>, 256U>(nums.begin(), nums.end(), {});
        return *(nums.rbegin() + k - 1);
    }

    template<typename T>
    struct identity_key_extractor {
        template<typename U>
        constexpr auto operator()(U&& value)
            -> std::enable_if_t<std::is_same_v<std::decay_t<U>, T>, T>
        {
            return std::forward<U>(value);
        }
    };

    template<typename ContigIter, typename Compare, typename KeyExtractor, std::uint32_t _Bucket_size>
    requires((std::is_same_v<Compare, std::less<>> || std::is_same_v<Compare, std::greater<>>) &&
             (_Bucket_size == 256U || _Bucket_size == 65536U))
    void radixsort(ContigIter _First, ContigIter _Last, KeyExtractor&& _Extractor) {
        static_assert(std::contiguous_iterator<ContigIter>, 
            "Radix sort requires that the iterator must be contiguous! Like vector, array, raw point");

        using Value_t = typename std::iter_value_t<ContigIter>;
        using Key_t = std::decay_t<decltype(_Extractor(std::declval<Value_t>()))>;
        static_assert(std::is_arithmetic_v<Key_t> && !std::is_same_v<Key_t, bool>, 
            "Key type must be arithmetic( not bool )! Like Integer or Floating");

        static_assert(!(sizeof(Key_t) == 1 && _Bucket_size == 65536U), "1-Bytes bucket size should not be set 65536U!");

        using Unsigend_t = std::make_unsigned_t<
            std::conditional_t<
                std::is_floating_point_v<Key_t>,
                std::conditional_t<sizeof(Key_t) == 4, std::uint32_t, std::uint64_t>,
                Key_t
            >
        >;
        
        Value_t* _Ptr = &*_First;
        std::size_t _Size = std::distance(_First, _Last);
        if (_Size <= 1) return;

        constexpr std::uint8_t _Passes = _Bucket_size == 256U ? sizeof(Key_t) : sizeof(Key_t) >> 1;
        constexpr std::uint8_t _Shift = _Bucket_size == 256U ? 3 : 4;
        constexpr std::uint16_t _Mask = _Bucket_size - 1;

        std::array<std::size_t, _Bucket_size> _Bucket_count;
        std::array<std::size_t, _Bucket_size> _Scanned;

        std::vector<Value_t> _Buffer(_Size);
        Value_t* __restrict _Start = _Ptr;
        Value_t* __restrict _End = _Buffer.data(); 

        for (std::uint8_t _Pass = 0; _Pass < _Passes; ++_Pass) {
            std::fill(_Bucket_count.begin(), _Bucket_count.end(), 0);

            #pragma clang loop vectorize(enable) unroll_count(8)
            for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
                Unsigend_t _Unsigned_value = std::bit_cast<Unsigend_t>(_Extractor(_Start[_Idx]));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else if constexpr (sizeof(Key_t) == 8)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
                    }
                    else {
                        if constexpr (sizeof(Key_t) == 1) _Unsigned_value ^= 0x80U;
                        else if constexpr (sizeof(Key_t) == 2) _Unsigned_value ^= 0x8000U;
                        else if constexpr (sizeof(Key_t) == 4) _Unsigned_value ^= 0x8000'0000U;
                        else if constexpr (sizeof(Key_t) == 8) _Unsigned_value ^=  0x8000'0000'0000'0000ULL;
                    }
                }

                std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

                if constexpr (std::is_same_v<Compare, std::greater<>>) _Byte_idx = _Mask - _Byte_idx;

                ++_Bucket_count[_Byte_idx];
            }

            std::exclusive_scan(_Bucket_count.begin(), _Bucket_count.end(), _Scanned.begin(), 0, std::plus<>{});

            #pragma clang loop vectorize(enable) unroll_count(8)
            for (std::size_t _Idx = 0; _Idx < _Size; ++_Idx) {
                auto _Value = std::move(_Start[_Idx]);
                Unsigend_t _Unsigned_value = std::bit_cast<Unsigend_t>(_Extractor(_Value));

                if constexpr (std::is_signed_v<Key_t>) {
                    if constexpr (std::is_floating_point_v<Key_t>) {
                        if constexpr (sizeof(Key_t) == 4)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000U) ? 0xFFFF'FFFFU : 0x8000'0000U;
                        else if constexpr (sizeof(Key_t) == 8)
                            _Unsigned_value ^= (_Unsigned_value & 0x8000'0000'0000'0000ULL) ? 0xFFFF'FFFF'FFFF'FFFFULL : 0x8000'0000'0000'0000ULL;
                    }
                    else {
                        if constexpr (sizeof(Key_t) == 1) _Unsigned_value ^= 0x80U;
                        else if constexpr (sizeof(Key_t) == 2) _Unsigned_value ^= 0x8000U;
                        else if constexpr (sizeof(Key_t) == 4) _Unsigned_value ^= 0x8000'0000U;
                        else if constexpr (sizeof(Key_t) == 8) _Unsigned_value ^=  0x8000'0000'0000'0000ULL;
                    }
                }

                std::uint16_t _Byte_idx = (_Unsigned_value >> (_Pass << _Shift)) & _Mask;

                if constexpr (std::is_same_v<Compare, std::greater<>>) _Byte_idx = _Mask - _Byte_idx;

                _End[_Scanned[_Byte_idx]++] = std::move(_Value);
            }

            std::swap(_Start, _End);
        }

        if (sizeof(Key_t) == 1) std::move(_Start, _Start + _Size, _Ptr);
    }
    
#include <queue>
#include <algorithm>
#include <array>
using namespace std;
// @lc code=end 
/*
 * @lc app=leetcode.cn id=215 lang=cpp
 *
 * [215] 数组中的第K个最大元素
 */

// @lc code=start
class Solution {
public:
    int findKthLargest(vector<int>& nums, int k) {
//#define MIN_HEAP
//#define QUICK_SELCT
//#define SORT
#define COUNT_SORT
#ifdef MIN_HEAP
        priority_queue<int, vector<int>, greater<int>> min_heap; // 最小堆
        for (int num : nums) {
            if (min_heap.size() < k) {
                min_heap.push(num);
            } else if (num > min_heap.top()) {
                min_heap.pop();
                min_heap.push(num);
            }
        }
        return min_heap.top();
#elif defined(QUICK_SELCT)
        // 将第k大转换为第(n - k + 1)小
        int n = nums.size();
        int target_pos = n - k;
        int left = 0, right = n - 1;
        
        while (true) {
            int pivot_index = partition(nums, left, right);
            if (pivot_index == target_pos) {
                return nums[pivot_index];
            } else if (pivot_index < target_pos) {
                left = pivot_index + 1;
            } else {
                right = pivot_index - 1;
            }
        }
#elif defined(SORT)
        sort(nums.begin(), nums.end(), greater<int>());
        return nums[k - 1];
#elif defined(COUNT_SORT)
        array<int, 20001> count{0};
        #pragma GCC unroll 8
        for(int n : nums) ++count[n + 10000]; // 正确统计次数
        #pragma GCC unroll 8
        for(int i = 20000; i >=0; --i){
            if(count[i] == 0) continue;
            // 当剩余k小于当前元素个数时，即为目标值
            if(k > count[i]) {
                k -= count[i];
            } else {
                return i - 10000; // 回撤偏移量
            }
        }
        return -1;
#endif
    }

private:
    int partition(vector<int>& nums, int left, int right) {
        // 随机选择pivot避免最坏情况
        int pivot_index = left + rand() % (right - left + 1);
        int pivot = nums[pivot_index];
        swap(nums[pivot_index], nums[right]); // 将pivot移到末尾
        
        int store_index = left;
        for (int i = left; i < right; ++i) {
            if (nums[i] < pivot) {
                swap(nums[i], nums[store_index]);
                store_index++;
            }
        }
        swap(nums[store_index], nums[right]); // 将pivot放回正确位置
        return store_index;
    }
};
// @lc code=end

