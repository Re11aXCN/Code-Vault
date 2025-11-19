/*
 * @lc app=leetcode.cn id=199 lang=cpp
 *
 * [199] 二叉树的右视图
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
    std::vector<int> rightSideView(TreeNode* root) {
        if (!root) return {};

        std::vector<int> res;
        res.reserve(11);

        std::queue<TreeNode*> q;
        q.push(root);
        
        while (!q.empty())
        {
            int qSize = q.size();
            for (int i = 0; i < qSize; ++i) {
                TreeNode* node = q.front();
                q.pop();

                if (!i) res.push_back(node->val);

                if (node->right) q.push(node->right);
                if (node->left) q.push(node->left);
                
            }
        }
        return res;
    }

    vector<int> rightSideView(TreeNode* root) {
        vector<int> result; // 存储结果
        
        // 如果根节点为空，直接返回空结果
        if (!root) return result;
        
        // 使用队列进行层序遍历
        queue<TreeNode*> q;
        q.push(root);
        
        // 层序遍历
        while (!q.empty()) {
            int size = q.size(); // 当前层的节点数
            
            // 遍历当前层的所有节点
            for (int i = 0; i < size; i++) {
                TreeNode* node = q.front();
                q.pop();
                
                // 如果是当前层的最后一个节点，将其值加入结果
                if (i == size - 1) result.push_back(node->val);
                
                // 将左右子节点加入队列，注意先左后右
                if (node->left) q.push(node->left);
                if (node->right) q.push(node->right);
            }
        }
        
        return result;
    }
};
// @lc code=end

