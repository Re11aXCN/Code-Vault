/* 
 * @lc app=leetcode.cn id=46 lang=cpp
 * 
 * [46] 全排列 
 */

// @lc code=start
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    vector<vector<int>> permute(vector<int>& nums) {
#ifdef RECURSION_BACKTRACKING
        // 递归回溯法
        vector<vector<int>> result;
        vector<int> path;
        vector<bool> used(nums.size(), false);
        backtrack(nums, path, used, result);
        return result;
#elif defined(SWAP_METHOD)
        // 交换法
        vector<vector<int>> result;
        permuteSwap(nums, 0, result);
        return result;
#else
        // STL算法法
        vector<vector<int>> result;
        // 先排序，确保从字典序最小的排列开始,也是基于交换，但是无vector的参数拷贝
        sort(nums.begin(), nums.end());
        do {
            result.push_back(nums);
        } while (next_permutation(nums.begin(), nums.end()));
        return result;
#endif
    }

private:
    // 递归回溯法辅助函数
    void backtrack(const vector<int>& nums, vector<int>& path, vector<bool>& used, vector<vector<int>>& result) {
        // 如果路径长度等于数组长度，说明找到了一个排列
        if (path.size() == nums.size()) {
            result.push_back(path);
            return;
        }
        
        // 尝试选择每个数字
        for (int i = 0; i < nums.size(); i++) {
            // 如果当前数字已经使用过，则跳过
            if (used[i]) continue;
            
            // 选择当前数字
            path.push_back(nums[i]);
            used[i] = true;
            
            // 递归生成剩余数字的排列
            backtrack(nums, path, used, result);
            
            // 回溯，撤销选择
            used[i] = false;
            path.pop_back();
        }
    }
    
    // 交换法辅助函数
    void permuteSwap(vector<int> nums, int start, vector<vector<int>>& result) {
        // 如果已经处理到最后一个位置，将当前排列加入结果
        if (start == nums.size() - 1) {
            result.push_back(std::move(nums));
            return;
        }
        
        // 尝试将当前位置与后面的每个位置交换
        for (int i = start; i < nums.size(); i++) {
            // 交换当前位置与位置i的元素
            swap(nums[start], nums[i]);
            
            // 递归处理下一个位置
            permuteSwap(nums, start + 1, result);
            
            // 恢复交换（回溯）
            // 注意：这里不需要真正交换回来，因为我们传递的是nums的副本
            // 但为了代码的完整性和理解，保留这一步
            // swap(nums[start], nums[i]);
        }
    }
};
// @lc code=end