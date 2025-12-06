
#include <vector>
using std::vector;
/*
 * @lc app=leetcode.cn id=95 lang=cpp
 *
 * [95] 不同的二叉搜索树 II
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
    vector<TreeNode*> generateTrees(int n) {
        if (n == 0) return {};
        
        // dp[i] 表示由 i 个节点组成的所有不同的二叉搜索树
        vector<vector<TreeNode*>> dp(n + 1);
        dp[0] = {nullptr}; // 空树
        
        for (int len = 1; len <= n; len++) { // 树的大小从1到n
            vector<TreeNode*> trees;
            
            for (int rootVal = 1; rootVal <= len; ++rootVal) {
                // 左子树：由 1 到 rootVal-1 组成，大小为 rootVal-1
                vector<TreeNode*>& leftTrees = dp[rootVal - 1];
                // 右子树：由 rootVal+1 到 len 组成，大小为 len-rootVal
                // 注意：右子树的节点值需要偏移，但结构相同
                vector<TreeNode*>& rightTrees = dp[len - rootVal];
                
                // 遍历所有可能的左右子树组合
                for (TreeNode* left : leftTrees) {
                    for (TreeNode* right : rightTrees) {
                        TreeNode* root = new TreeNode(rootVal);
                        root->left = left;
                        // 克隆右子树并调整节点值
                        root->right = cloneTree(right, rootVal);
                        trees.push_back(root);
                    }
                }
            }
            dp[len] = trees;
        }
        
        return dp[n];
    }
    
private:
    // 克隆树并给所有节点值加上偏移量
    TreeNode* cloneTree(TreeNode* root, int offset) {
        if (!root) return nullptr;
        
        TreeNode* newRoot = new TreeNode(root->val + offset);
        newRoot->left = cloneTree(root->left, offset);
        newRoot->right = cloneTree(root->right, offset);
        
        return newRoot;
    }
};
// @lc code=end
class Solution {
private:
    size_t catalan(int n) {
        if (n == 0) return 1;
        float log_comb = std::lgammaf(2 * n + 1) - 2 * std::lgammaf(n + 1);
        return std::expf(log_comb + 1e-5f) / (n + 1.0f);
    }

    // 递归生成[start, end]范围内的所有BST
    std::vector<TreeNode*> generate(int start, int end) {
        if (start > end) return {nullptr};
        
        std::vector<TreeNode*> res;
        res.reserve(catalan(end - start + 1));

        // 遍历每个可能的根节点
        for (int rootVal = start; rootVal <= end; ++rootVal) {
            // 生成所有可能的左子树（由较小的数字组成）
            std::vector<TreeNode*> leftTrees = generate(start, rootVal - 1);
            // 生成所有可能的右子树（由较大的数字组成）
            std::vector<TreeNode*> rightTrees = generate(rootVal + 1, end);

            // 组合左、根、右
            for (TreeNode* left : leftTrees) {
                for (TreeNode* right : rightTrees) {
                    TreeNode* root = new TreeNode(rootVal);
                    root->left = left;
                    root->right = right;
                    res.push_back(root);
                }
            }
        }
        return res;
    }

public:
    std::vector<TreeNode*> generateTrees(int n) {
        if (n == 0) return {};
        std::vector<TreeNode*> result = generate(1, n);
        return result;
    }
};