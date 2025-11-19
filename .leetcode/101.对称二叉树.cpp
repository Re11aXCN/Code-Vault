class Solution {
public:
    bool isSymmetric(TreeNode* root) {
        if (root == nullptr) return true;
        
        deque<TreeNode*> dq;
        // 初始加入根节点的左右子节点
        dq.push_back(root->left);
        dq.push_back(root->right);
        
        while (!dq.empty()) {
            // 每次取出对称位置的两个节点
            TreeNode* leftNode = dq.front(); dq.pop_front();
            TreeNode* rightNode = dq.front(); dq.pop_front();
            
            // 两个节点都为空 - 对称，继续
            if (leftNode == nullptr && rightNode == nullptr) {
                continue;
            }
            
            // 一个为空一个不为空 - 不对称
            if (leftNode == nullptr || rightNode == nullptr) {
                return false;
            }
            
            // 节点值不相等 - 不对称
            if (leftNode->val != rightNode->val) {
                return false;
            }
            
            // 按镜像对称的顺序加入子节点对：
            // 左子树的左节点 vs 右子树的右节点
            dq.push_back(leftNode->left);
            dq.push_back(rightNode->right);
            
            // 左子树的右节点 vs 右子树的左节点
            dq.push_back(leftNode->right);
            dq.push_back(rightNode->left);
        }
        
        return true;
    }
};
/*
 * @lc app=leetcode.cn id=101 lang=cpp
 *
 * [101] 对称二叉树
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
    //两个指针同时遍历
    bool check(TreeNode *p, TreeNode *q) {
        if (!p && !q) return true;
        if (!p || !q) return false;
        return p->val == q->val && check(p->left, q->right) && check(p->right, q->left);
    }

    bool isSymmetric(TreeNode* root) {
        return check(root->left, root->right);
    }
};
// @lc code=end

