/*
 * @lc app=leetcode.cn id=98 lang=cpp
 *
 * [98] 验证二叉搜索树
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
    bool isValidBST(TreeNode* root) {
        return helper(root, LONG_MIN, LONG_MAX);
    }
    
private:
    bool helper(TreeNode* node, long lower, long upper) {
        if (!node) return true;
        // 当前节点值必须严格大于下限，严格小于上限
        if (node->val <= lower || node->val >= upper) return false;
        // 左子树的上限是当前节点值，右子树的下限是当前节点值
        return helper(node->left, lower, node->val) && 
               helper(node->right, node->val, upper);
    }
};
// @lc code=end

