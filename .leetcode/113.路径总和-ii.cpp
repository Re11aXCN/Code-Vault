/*
 * @lc app=leetcode.cn id=113 lang=cpp
 *
 * [113] 路径总和 II
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
public:
    vector<vector<int>> pathSum(TreeNode* root, int targetSum) {
        vector<vector<int>> result;
        vector<int> currentPath;
        dfs(root, targetSum, currentPath, result);
        return result;
    }

private:
    void dfs(TreeNode* node, int remainingSum, vector<int>& currentPath, vector<vector<int>>& result) {
        if (!node) return;
        
        currentPath.push_back(node->val); // 将当前节点加入路径
        
        // 到达叶子节点且路径和满足条件
        if (!node->left && !node->right && remainingSum == node->val) {
            result.push_back(currentPath);
        }
        
        // 递归处理左右子树
        dfs(node->left, remainingSum - node->val, currentPath, result);
        dfs(node->right, remainingSum - node->val, currentPath, result);
        
        currentPath.pop_back(); // 回溯，移除当前节点
    }
};
// @lc code=end

