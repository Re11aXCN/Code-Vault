/*
 * @lc app=leetcode.cn id=104 lang=cpp
 *
 * [104] 二叉树的最大深度
 */

// 三种情况到达curr，左右父亲到达，Morris设置的左孩子的最右节点的right 指向curr

// 左右孩子向下，++preLevel
// curr 指向 Morris设置的左孩子的最右节点的right，则curr的depth 等于 最右节点的depth - curr到达最右节点的rightLen它们的路径长度
// 第二次来到子树的根的时候，能够判断 mostRight是不是叶子节点

class Solution {
public:
    int maxDepth(TreeNode* root) {
        // 处理空树的情况
        if (root == nullptr) return 0;
        
        int maxDepth = 0;        // 记录最大深度
        int currentDepth = 0;    // 当前节点的深度
        int rightPathLength = 0; // 右路径长度
        TreeNode* curr = root;
        TreeNode* predecessor = nullptr;
        
        while (curr != nullptr) {
            if (curr->left == nullptr) {
                // 没有左子树，直接转向右子树
                ++currentDepth;
                // 更新最大深度
                if (currentDepth > maxDepth) {
                    maxDepth = currentDepth;
                }
                curr = curr->right;
            }
            else {
                // 找到当前节点在中序遍历下的前驱节点
                rightPathLength = 1;
                predecessor = curr->left;
                
                // 寻找左子树的最右节点
                while (predecessor->right != nullptr && predecessor->right != curr) {
                    predecessor = predecessor->right;
                    ++rightPathLength;
                }
                
                if (predecessor->right == nullptr) {
                    // 第一次访问，建立临时链接
                    predecessor->right = curr;
                    ++currentDepth;
                    // 更新最大深度
                    if (currentDepth > maxDepth) {
                        maxDepth = currentDepth;
                    }
                    curr = curr->left;
                }
                else {
                    // 第二次访问，断开临时链接
                    predecessor->right = nullptr;
                    // 调整深度并转向右子树
                    currentDepth -= rightPathLength;
                    curr = curr->right;
                }
            }
        }
        
        return maxDepth;
    }
};
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
    int maxDepth(TreeNode* root) {
       if (root == nullptr) return 0;
       return max(maxDepth(root->left), maxDepth(root->right)) + 1;
    }
};
// @lc code=end

