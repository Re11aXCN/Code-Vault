/*
 * @lc app=leetcode.cn id=103 lang=cpp
 *
 * [103] 二叉树的锯齿形层序遍历
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
#include <queue>
#include <vector>
using std::queue;
using std::vector;
class Solution {
public:
    vector<vector<int>> zigzagLevelOrder(TreeNode* root) {
        if(!root) return {};
        
        vector<vector<int>> res;
        res.reserve(11);
        queue<TreeNode*> q;
        q.emplace(root);
        int layer = 1;
        while(!q.empty()) {
            int size = q.size();
            vector<int> layerVec;
            for(int i = 0; i < size; ++i) {
                TreeNode* node = q.front();
                q.pop();
                layerVec.push_back(node->val);

                if(node->left) q.push(node->left);
                if(node->right) q.push(node->right);
            }
            if(layer & 1) {
                res.push_back(std::move(layerVec));
            }
            else
            {
                vector<int> temp;
                temp.reserve(layerVec.size());
                std::move(layerVec.rbegin(), layerVec.rend(), std::back_inserter(temp));
                res.push_back(std::move(temp));
            }
            ++layer;
        }
        return res;
    }
};
// @lc code=end

