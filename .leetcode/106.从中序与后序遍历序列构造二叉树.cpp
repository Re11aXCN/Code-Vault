/*
 * @lc app=leetcode.cn id=106 lang=cpp
 *
 * [106] 从中序与后序遍历序列构造二叉树
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
    TreeNode* buildTree(vector<int>& inorder, vector<int>& postorder) {
        return build(inorder, 0, inorder.size()-1, 
                    postorder, 0, postorder.size()-1);
    }
    
private:
    TreeNode* build(vector<int>& in, int inStart, int inEnd,
                    vector<int>& post, int postStart, int postEnd) {
        if (inStart > inEnd || postStart > postEnd) return nullptr;
        
        TreeNode* root = new TreeNode(post[postEnd]); // 后序末元素为根
        
        int inRoot = inStart;
        while (in[inRoot] != root->val) ++inRoot; // 找到中序中的根位置
        
        int leftSubtreeSize = inRoot - inStart; // 左子树节点数
        
        // 递归构建左子树：后序左范围[postStart, postStart+leftSubtreeSize-1]
        root->left = build(in, inStart, inRoot-1, 
                          post, postStart, postStart+leftSubtreeSize-1);
        // 递归构建右子树：后序右范围[postStart+leftSubtreeSize, postEnd-1]
        root->right = build(in, inRoot+1, inEnd,
                           post, postStart+leftSubtreeSize, postEnd-1);
        return root;
    }
};
// @lc code=end

