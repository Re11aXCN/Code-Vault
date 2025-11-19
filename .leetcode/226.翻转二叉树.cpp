class Solution {
public:
    TreeNode* invertTree(TreeNode* root) {
        if (!root) return root;
        traversal(root);
        return root;
    }

    void traversal(TreeNode* root)
    {
        if(!root) return;

        traversal(root->left);
        traversal(root->right);
        std::swap(root->left, root->right);
    }
};
/*
 * @lc app=leetcode->cn id=226 lang=cpp
 *
 * [226] 翻转二叉树
 */

// @lc code=start
/**
 * Definition for a binary tree node->
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
    TreeNode* invertTree(TreeNode* root) {
        if(!root) return nullptr;
		TreeNode* tmp = root->right;
		root->right = root->left;
		root->left = tmp;

		//递归交换当前节点的 左子树
		invertTree(root->left);
		//递归交换当前节点的 右子树
		invertTree(root->right);
		return root;
    }
};
// @lc code=end

