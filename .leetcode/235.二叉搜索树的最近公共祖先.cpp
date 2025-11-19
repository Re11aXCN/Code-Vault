/*
 * @lc app=leetcode.cn id=235 lang=cpp
 *
 * [235] 二叉搜索树的最近公共祖先
 */

// @lc code=start
/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */

class Solution {
public:
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
          // 如果根节点为空，返回NULL
        if (root == NULL) return NULL;
        
        // 获取p和q的值
        int pVal = p->val;
        int qVal = q->val;
        
        // 如果p和q的值都小于根节点的值
        // 说明p和q都在根节点的左子树中
        // 递归在左子树中寻找最近公共祖先
        if (pVal < root->val && qVal < root->val) {
            return lowestCommonAncestor(root->left, p, q);
        }
        
        // 如果p和q的值都大于根节点的值
        // 说明p和q都在根节点的右子树中
        // 递归在右子树中寻找最近公共祖先
        if (pVal > root->val && qVal > root->val) {
            return lowestCommonAncestor(root->right, p, q);
        }
        
        // 如果p和q的值一个大于根节点值，一个小于根节点值
        // 或者其中一个等于根节点值
        // 说明当前根节点就是p和q的最近公共祖先
        return root;
    }
};
// @lc code=end

