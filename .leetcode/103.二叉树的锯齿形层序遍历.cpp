    vector<vector<int>> zigzagLevelOrder(TreeNode* root) {
        if (!root) return {};
        std::vector<std::vector<int>> result; result.reserve(4);

        std::vector<TreeNode*> queue; queue.reserve(8);
        int left = 0, right = 1;
        queue.emplace_back(root);
        int levelNum = 1;
        while(left != queue.size()) {
            int size = right - left;
            std::vector<int> levelVec; levelVec.reserve(/*std::pow(2, levelNum - 1)*/ right - left);
            if (levelNum++ & 1) {
                std::transform(queue.begin() + left, queue.end(), std::back_inserter(levelVec), [](TreeNode* n) { return n->val; });
                result.push_back(std::move(levelVec));
            }
            else {
                std::transform(queue.rbegin(), queue.rbegin() + size, std::back_inserter(levelVec), [](TreeNode* n) { return n->val; });
                result.push_back(std::move(levelVec));
            }
            int oldLeft = left;
            left = right;
            while (oldLeft != left) {
                TreeNode* node = queue[oldLeft++];
                if (node->left) {
                    queue.emplace_back(node->left);
                    ++right;
                }
                if (node->right) {
                    queue.emplace_back(node->right);
                    ++right;
                }
            }
        }
        
        return result;
    }
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

