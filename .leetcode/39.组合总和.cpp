/*
 * @lc app=leetcode.cn id=39 lang=cpp
 *
 * [39] 组合总和
 */

// @lc code=start
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
template<typename BiIter, typename Compare>
    void insert_sort(BiIter start, BiIter end, Compare comp){
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
    vector<vector<int>> combinationSum(vector<int>& candidates, int target) {
        // 先对候选数组排序，便于剪枝
        insert_sort(candidates.begin(), candidates.end(), std::less<>{});
        
        std::vector<std::vector<int>> result; // 存储最终结果
        std::vector<int> subSet; // 当前路径
        // 调用回溯函数
        backtrack(candidates, result, subSet, target, 0);
        return result;
    }

private:
    // 回溯函数
    // candidates: 候选数组
    // target: 当前的目标值
    // start: 当前考虑的起始位置（避免重复组合）
    // path: 当前路径
    // result: 结果集
     void backtrack(const std::vector<int> candidates, std::vector<std::vector<int>>& result, std::vector<int>& subSet,
        int target, int start)
    {
        // 如果目标值为0，说明当前路径的和等于原目标值，找到一个解
        if (target == 0) {
            result.push_back(subSet);
            return;
        }
        
        // 从start开始遍历候选数组
        for (int i = start; i < candidates.size(); i++) {
            // 剪枝：如果当前数字已经大于目标值，由于数组已排序，后面的数字更大，无需继续
            const auto& candiVal = candidates[i];
            if(candiVal > target) break;
            
            // 选择当前数字
            subSet.push_back(candiVal);
            
            // 递归，注意这里传入的起始位置仍为i，因为每个数字可以重复使用
            // 目标值减去当前数字
             backtrack(candidates, result, subSet, target - candiVal, i);
            
            // 回溯，撤销选择
            subSet.pop_back();
        }
    }
};
// @lc code=end