// 返回尾节点的递归
class Solution {
public:
    void flatten(TreeNode* root) {
        flattenHelper(root);
    }
    
private:
    TreeNode* flattenHelper(TreeNode* root) {
        if (!root) return nullptr;
        
        // 如果是叶子节点，直接返回
        if (!root->left && !root->right) {
            return root;
        }
        
        TreeNode* leftTail = flattenHelper(root->left);
        TreeNode* rightTail = flattenHelper(root->right);
        
        // 如果左子树存在，将其插入到根节点和右子树之间
        if (leftTail) {
            leftTail->right = root->right;
            root->right = root->left;
            root->left = nullptr;
        }
        
        // 返回链表的尾节点
        return rightTail ? rightTail : leftTail;
    }
};
// 后序遍历递归（展开左子树 → 展开右子树 → 重新连接），效率低于返回尾节点的递归
class Solution {
public:
    void flatten(TreeNode* root) {
        if (!root) return;
        
        // 递归展开左右子树
        flatten(root->left);
        flatten(root->right);
        
        // 保存右子树
        TreeNode* right = root->right;
        
        // 将左子树移到右边
        root->right = root->left;
        root->left = nullptr;
        
        // 找到当前链表的末尾，接上原来的右子树
        TreeNode* curr = root;
        while (curr->right) {
            curr = curr->right;
        }
        curr->right = right;
    }
};
/*
 * @lc app=leetcode.cn id=114 lang=cpp
 *
 * [114] 二叉树展开为链表
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
    void flatten(TreeNode* root) {
        TreeNode* curr = root;
        while (curr) {
            if (TreeNode* predecessor = curr->left) {
                // 找到左子树的最右节点（前驱节点）
                // 因为我们的三步swap操作 前驱节点的右指针一定是 nullptr，所以不需要再判断
                while (predecessor->right /*&& predecessor->right != curr*/) predecessor = predecessor->right;
                
                // 将右子树接在前驱节点右侧
                predecessor->right = curr->right;
                // 左子树移到右侧，左指针置空
                curr->right = curr->left;
                curr->left = nullptr;
            }
            curr = curr->right; // 处理下一个节点
        }
    }
};
// @lc code=end
