void recoverTree(TreeNode* root) {
    TreeNode *prev = nullptr, *first = nullptr, *second = nullptr;
    TreeNode *curr = root;
    
    while (curr) {
        TreeNode *predecessor = curr->left;
        if (predecessor) {
            // 找到当前节点的前驱节点
            while (predecessor->right && predecessor->right != curr) {
                predecessor = predecessor->right;
            }
            
            if (!predecessor->right) {
                // 建立临时链接，然后向左移动
                predecessor->right = curr;
                curr = curr->left;
            } 
            else {
                // 断开临时链接，处理当前节点
                predecessor->right = nullptr;
                
                // 检查逆序对
                if (prev && prev->val > curr->val) {
                    if (!first) first = prev;
                    second = curr;
                }
                prev = curr;
                
                curr = curr->right;
            }
        } 
        else {
            // 没有左子树，直接处理当前节点
            if (prev && prev->val > curr->val) {
                if (!first) first = prev;
                second = curr;
            }
            prev = curr;
            
            curr = curr->right;
        }
    }
    
    // 交换两个错误节点的值
    if (first && second) {
        std::swap(first->val, second->val);
    }
}

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

