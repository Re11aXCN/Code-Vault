/*
 * @lc app=leetcode.cn id=90 lang=cpp
 *
 * [90] 子集 II
 */

// @lc code=start
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
    std::vector<std::vector<int>> subsetsWithDup(std::vector<int>& nums) {
        insert_sort(nums.begin(), nums.end(), std::less<>{});

        std::vector<std::vector<int>> result;
        std::vector<int> currSet;
        
        backtrack(nums, result, currSet, 0);
        return result;
    }
    void backtrack(const vector<int>& nums, std::vector<std::vector<int>>& result, std::vector<int>& currSet, int start) {
        // 将当前子集加入结果
        result.push_back(currSet);
        
        for (int i = start; i < nums.size(); ++i) {
            // 剪纸 跳过重复元素
            // 如果当前元素与前一个元素相同，且不是第一个考虑的选项，则跳过
            if (i > start && nums[i] == nums[i - 1])continue;
            
            // 选择当前元素
            currSet.emplace_back(nums[i]);
            
            // 递归处理下一个元素
            backtrack(nums, result, currSet, i + 1);

            // 回溯，撤销选择
            currSet.pop_back();
        }
    }
};
// @lc code=end

