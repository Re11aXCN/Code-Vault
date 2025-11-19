/*
 * @lc app=leetcode.cn id=105 lang=cpp
 *
 * [105] 从前序与中序遍历序列构造二叉树
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
    TreeNode* buildTree(vector<int>& preorder, vector<int>& inorder) {
        return build(preorder, 0, preorder.size() - 1, inorder, 0, inorder.size() - 1);
    }
    
private:
    TreeNode* build(vector<int>& pre, int preStart, int preEnd, 
                    vector<int>& in, int inStart, int inEnd) {
        if (preStart > preEnd || inStart > inEnd) return nullptr;
        
        TreeNode* root = new TreeNode(pre[preStart]); // 前序首元素为根
        
        // 在中序中找到根的位置
        int inRoot = inStart;
        while (in[inRoot] != root->val) ++inRoot;
        
        int leftSubtreeSize = inRoot - inStart; // 左子树节点数
        
        // 递归构建左子树：前序左范围[preStart+1, preStart+leftSubtreeSize]
        root->left = build(pre, preStart + 1, preStart + leftSubtreeSize, 
                          in, inStart, inRoot - 1);
        // 递归构建右子树：前序右范围[preStart+leftSubtreeSize+1, preEnd]
        root->right = build(pre, preStart + leftSubtreeSize + 1, preEnd,
                           in, inRoot + 1, inEnd);
        return root;
    }
};
// @lc code=end

