/*
 * @lc app=leetcode.cn id=337 lang=cpp
 *
 * [337] 打家劫舍 III
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
    int rob(TreeNode* root) {
        auto [root_rob, root_not_rob] = dfs(root);
        return std::max(root_rob, root_not_rob); // 根节点选或不选的最大值
    }

    // 后续遍历，自底向上需要传递状态
    std::pair<int, int> dfs(TreeNode* node) {
        if (node == nullptr) { // 递归边界
            return {0, 0}; // 没有节点，怎么选都是 0
        }

        auto [l_rob, l_not_rob] = dfs(node->left); // 递归左子树
        auto [r_rob, r_not_rob] = dfs(node->right); // 递归右子树
        int rob = l_not_rob + r_not_rob + node->val; // 选当前，加上先前值
        // **当前不偷**的情况，意味着左右有间隔，从**左右孩子**选择偷与不偷的最大相加
        int not_rob = std::max(l_rob, l_not_rob) + std::max(r_rob, r_not_rob); // 不选

        return {rob, not_rob};
    }
};
// @lc code=end

