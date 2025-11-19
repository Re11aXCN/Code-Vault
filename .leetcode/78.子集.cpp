/*
 * @lc app=leetcode.cn id=78 lang=cpp
 *
 * [78] 子集
 */

// @lc code=start
class Solution {
public:
    vector<vector<int>> subsets(vector<int>& nums) {
#ifndef BACKTRACK 
        std::vector<std::vector<int>> result;
        result.reserve(std::pow(2, nums.size()));
        // 初始时只有一个空集
        result.push_back({});
        
        // 遍历数组中的每个元素
        for (int num : nums) {
            // 记录当前结果集的大小
            int n = result.size();
            // 遍历当前结果集中的每个子集
            for (int i = 0; i < n; i++) {
                // 复制当前子集
                std::vector<int> newSubset = result[i];
                // 将当前元素加入到子集中，形成新的子集
                newSubset.push_back(num);
                // 将新子集加入到结果集中
                result.push_back(newSubset);
            }
        }
        
        return result;
#else
        std::vector<std::vector<int>> result;
        std::vector<int> currSet;
        backtrack(nums, result, currSet, 0);
        return result;
#endif
    }
private:
    // 回溯函数，index表示当前处理的元素索引，current是当前构建的子集
    void backtrack(const vector<int>& nums, std::vector<std::vector<int>>& result, std::vector<int>& currSet, int start) {
        // 将当前子集加入结果（包括空集）
        result.push_back(currSet);

        for (int i = start; i < nums.size(); ++i) {
            // 选择当前元素
            currSet.emplace_back(nums[i]);

            backtrack(nums, result , currSet, i + 1);

            // 回溯，撤销选择
            currSet.pop_back();
        }
    }
};
// @lc code=end

