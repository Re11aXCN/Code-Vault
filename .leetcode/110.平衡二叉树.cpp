/*
 * @lc app=leetcode.cn id=110 lang=cpp
 *
 * [110] 平衡二叉树
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
    bool isBalanced(TreeNode* root) {
        return checkHeight(root) != -1;
    }
    
private:
    // 返回子树高度，若不平衡返回-1
    int checkHeight(TreeNode* node) {
        if (!node) return 0;
        
        int leftHeight = checkHeight(node->left);
        if (leftHeight == -1) return -1; // 左子树不平衡
        
        int rightHeight = checkHeight(node->right);
        if (rightHeight == -1) return -1; // 右子树不平衡
        
        if (abs(leftHeight - rightHeight) > 1) return -1; // 当前节点不平衡
        
        return max(leftHeight, rightHeight) + 1; // 返回当前子树高度
    }
};
// @lc code=end

