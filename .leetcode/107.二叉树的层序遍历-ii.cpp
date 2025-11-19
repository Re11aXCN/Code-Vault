    vector<vector<int>> levelOrderBottom(TreeNode* root) {
        if (!root) return {};
        std::vector<std::vector<int>> result; result.reserve(8);
        static std::vector<TreeNode*> q(2001, nullptr);
        q[0] = root;
        int left = 0, right = 1;

        while (left < right) {
            int currentLevelSize = right - left;
            std::vector<int> level;
            level.reserve(currentLevelSize);
            for (int i = 0; i < currentLevelSize; ++i) {
                TreeNode* node = q[left++];
                level.emplace_back(node->val);
                
                if (node->left) q[right++] = node->left;
                if (node->right) q[right++] = node->right;
            }
            result.push_back(std::move(level));
        }
        std::reverse(result.begin(), result.end());
        return result;
    }
/*
 * @lc app=leetcode.cn id=107 lang=cpp
 *
 * [107] 二叉树的层序遍历 II
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
    std::vector<std::vector<int>> levelOrderBottom(TreeNode* root) {
        if (!root) return {};

        std::vector<std::vector<int>> result;  result.reserve(16);
        
        std::queue<TreeNode*> q;
        q.push(root);
        
        while (!q.empty()) {
            int levelSize = q.size();
            std::vector<int> currentLevel; currentLevel.reserve(levelSize);
            for (int i = 0; i < levelSize; ++i) {
                TreeNode* node = q.front();
                q.pop();
                currentLevel.push_back(node->val);
                if (node->left) q.push(node->left);
                if (node->right) q.push(node->right);
            }
            result.push_back(std::move(currentLevel));
        }
        std::reverse(result.begin(), result.end()); // 反转结果
        return result;
    }
};
// @lc code=end

