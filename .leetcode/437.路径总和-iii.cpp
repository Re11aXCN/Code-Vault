/*
 * @lc app=leetcode.cn id=437 lang=cpp
 *
 * [437] 路径总和 III
 */

// @lc code=start
/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode() : val(0), left(nullptr), right(nullptr) {}
 *     TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
 *     TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
 * };
 */
class Solution {
public:
    int pathSum(TreeNode* root, int targetSum) {
        std::unordered_map<std::size_t, int> prefixSumCount; // 前缀和 -> 出现次数
        prefixSumCount.try_emplace(0, 1); // 初始化：前缀和为0的路径有1条（空路径）
        int count{0};
        dfs(root, targetSum, prefixSumCount, count, 0);
        return count;
    }

private:
    void dfs(TreeNode* root, int targetSum, std::unordered_map<std::size_t, int>& prefixSumCount,
            int& count, std::size_t currentSum)
    {
        if(!root) return;
        currentSum += root->val; // 更新当前路径前缀和

        // 检查是否存在前缀和满足 currentSum - prefixSum = target
        // 即 prefixSum = currentSum - target
        if(auto it = prefixSumCount.find(currentSum - targetSum); it != prefixSumCount.end())
        {
            count += it->second;
        }
        // 将当前前缀和加入哈希表
        prefixSumCount[currentSum]++;

        // 递归处理左右子树
        dfs(root->left, targetSum, prefixSumCount, count, currentSum);
        dfs(root->right, targetSum, prefixSumCount, count, currentSum);

        // 回溯：移除当前前缀和，避免影响其他子树 (栈要弹出 向上，不是直接删除sum而是--，是因为可能有其他相等的前缀和即使不是一个子树路径，减小hashmap插入删除的开销)
        if(prefixSumCount[currentSum]-- == 0) {
            prefixSumCount.erase(currentSum);
        }
    }
};
// @lc code=end

