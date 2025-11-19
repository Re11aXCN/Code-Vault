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
        
        for (int len = 1; ++len <= n; len) { // 树的大小从1到n
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
public:
    vector<TreeNode*> generateTrees(int n) {
        if (n == 0) return {};
        return generateTrees(1, n);
    }
    
private:
    vector<TreeNode*> generateTrees(int start, int end) {
        vector<TreeNode*> result;
        
        // 空树的情况
        if (start > end) {
            result.push_back(nullptr);
            return result;
        }
        
        // 遍历所有可能的根节点
        for (int i = start; i <= end; i++) {
            // 生成所有可能的左子树（由较小的数字组成）
            vector<TreeNode*> leftTrees = generateTrees(start, i - 1);
            
            // 生成所有可能的右子树（由较大的数字组成）
            vector<TreeNode*> rightTrees = generateTrees(i + 1, end);
            
            // 组合左右子树
            for (TreeNode* left : leftTrees) {
                for (TreeNode* right : rightTrees) {
                    TreeNode* root = new TreeNode(i);
                    root->left = left;
                    root->right = right;
                    result.push_back(root);
                }
            }
        }
        
        return result;
    }
};

