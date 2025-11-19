/*
 * @lc app=leetcode.cn id=99 lang=cpp
 *
 * [99] 恢复二叉搜索树
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
    void recoverTree(TreeNode* root) {
        TreeNode *first = nullptr, *second = nullptr, *prev = nullptr;
        inorder(root, prev, first, second); // 中序遍历找到错误节点
        swap(first->val, second->val);       // 交换值
    }

private:
// 为什么传入 指针的引用
//      需要修改指针本身：在每次递归调用中，我们需要更新 prev 指向当前节点
//      如果不传递引用：在递归返回时，上一层的 prev 不会更新，导致后续比较错误
    void inorder(TreeNode* node, TreeNode*& prev, TreeNode*& first, TreeNode*& second) {
        if (!node) return;
        
        // 递归遍历左子树
        inorder(node->left, prev, first, second);
        
        // 当前节点处理：检查逆序
        if (prev && prev->val > node->val) {
            if (!first) first = prev; // 第一个错误节点是prev
            second = node;            // 第二个错误节点是当前node，二叉搜索树性质中序遍历，我们应该找到最小的节点小于 first的值，所以需要不停更新
        }
        prev = node; // 更新prev
        
        // 递归遍历右子树
        inorder(node->right, prev, first, second);
    }
};
// @lc code=end

