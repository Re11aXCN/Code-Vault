/*
 * @lc app=leetcode.cn id=108 lang=cpp
 *
 * [108] 将有序数组转换为二叉搜索树
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
    TreeNode* sortedArrayToBST(vector<int>& nums) {
        return recursive(nums.begin(), std::prev(nums.end()));
    }
    //中序遍历
    template<class RanIter>
    TreeNode* recursive(RanIter left, RanIter right)
    {
        if(left > right) return nullptr;

        auto mid =  left + ((right - left) >> 1);

        TreeNode* node = new TreeNode(*mid, nullptr, nullptr);

        node->left = recursive(left, std::prev(mid));
        node->right = recursive(std::next(mid), right);

        return node;
    }
};
// @lc code=end

