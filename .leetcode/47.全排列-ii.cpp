/*
 * @lc app=leetcode.cn id=47 lang=cpp
 *
 * [47] 全排列 II
 */

// @lc code=start
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    template<typename BiIter, typename Compare>
    void insert_sort(BiIter start, BiIter end, Compare comp) {
        #pragma GCC unroll 8
        for(auto curr = std::next(start); curr != end; ++curr) {
            auto value = std::move(*curr);
            auto hole = curr;

            auto prev = hole;
            while(prev != start && comp(value, *std::prev(prev))) {
                --prev;
                *hole = std::move(*prev);
                hole = prev;
            }
            *hole = std::move(value);
        }
    }
    std::vector<std::vector<int>> permuteUnique(std::vector<int>& nums) {
        // 先排序，使相同的元素相邻，便于去重
        insert_sort(nums.begin(), nums.end(), std::less<>{});

        std::vector<std::vector<int>> result;
        std::vector<int> currSet;
        std::vector<uint8_t> used(nums.size(), 0);
        backtrack(nums, result, currSet, used);
        return result;
    }

    void backtrack(const std::vector<int>& nums, std::vector<std::vector<int>>& result, std::vector<int>& currSet,
        std::vector<uint8_t>& used)
    {
        // 如果路径长度等于数组长度，说明找到了一个排列
        if(nums.size() == currSet.size()) {
            result.push_back(currSet);
            return;
        }

        for(int i = 0; i < nums.size(); ++i) {
            auto& used_ref = used[i];
            if (used_ref) continue; // 防止重复使用同一个元素，跳过已使用的元素

            // [1 1 2]排列去重关键逻辑：
            // 如果当前元素与前一个元素相同，且前一个元素未被使用，则跳过
            // 这确保了对于相同的数字，我们按照它们在原数组中的顺序使用
            if (i > 0 && nums[i] == nums[i-1] && !used[i-1]) continue;

            currSet.emplace_back(nums[i]);
            used_ref = true;

            // 递归生成剩余数字的排列
            backtrack(nums, result, currSet, used);
            
            currSet.pop_back();
            used_ref = false;
        }
    }
};
// @lc code=end