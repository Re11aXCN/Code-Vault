bool hasPathSum(TreeNode* root, int targetSum) {
    std::stack<std::pair<TreeNode*, int>> stk;
    stk.emplace(root, targetSum);

    while(!stk.empty()) {
        auto [node, val] = stk.top(); stk.pop();

        if (!node) continue;

        if (!node->left && !node->right && val == node->val) return true;

        stk.emplace(node->left, val - node->val);
        stk.emplace(node->right, val - node->val);
    }

    return false;
}
/*
 * @lc app=leetcode.cn id=112 lang=cpp
 *
 * [112] 路径总和
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
    bool hasPathSum(TreeNode* root, int targetSum) {
        if (!root) return false;
        // 到达叶子节点时判断剩余值
        if (!root->left && !root->right) return (targetSum == root->val);
        // 递归检查左右子树
        return hasPathSum(root->left, targetSum - root->val) || 
               hasPathSum(root->right, targetSum - root->val);
    }
};
// @lc code=end

