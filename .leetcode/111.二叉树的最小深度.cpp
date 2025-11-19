
// BFS 层序遍历
class Solution {
public:
    int minDepth(TreeNode* root) {
        // 处理空树情况
        if (root == nullptr) return 0;
        
        // 使用队列进行BFS（广度优先搜索）
        // 队列中存储待处理的树节点
        std::queue<TreeNode*> q;
        q.push(root);  // 从根节点开始
        
        int depth = 1;  // 当前深度，根节点深度为1
        
        // 当队列不为空时继续处理
        while (!q.empty()) {
            // 记录当前层的节点数量
            int levelSize = q.size();
            
            // 遍历当前层的所有节点
            for (int i = 0; i < levelSize; ++i) {
                // 从队列头部取出一个节点
                TreeNode* node = q.front();
                q.pop();
                
                // 检查当前节点是否为叶子节点
                // 叶子节点：没有左子节点且没有右子节点
                if (node->left == nullptr && node->right == nullptr) {
                    return depth;  // 找到第一个叶子节点，立即返回当前深度
                    // 这是最小深度，因为BFS按层遍历，先遇到的叶子节点深度最小
                }
                
                // 如果当前节点不是叶子节点，将其非空子节点加入队列
                // 左子节点不为空，加入队列
                if (node->left != nullptr) {
                    q.push(node->left);
                }
                // 右子节点不为空，加入队列
                if (node->right != nullptr) {
                    q.push(node->right);
                }
            }
            
            // 完成当前层所有节点的处理，深度加1
            // 准备处理下一层节点
            ++depth;
        }
        
        return depth;  // 理论上不会执行到这里，因为循环中一定会遇到叶子节点
    }
};

// morris 遍历，无递归栈开销，但实际效率最低
class Solution {
public:
    int minDepth(TreeNode* root) {
        // 处理空树的情况
        if (root == nullptr) return 0;
        
        int minDepth = INT_MAX;  // 记录最小深度
        int currentDepth = 0;    // 当前节点的深度
        int rightPathLength = 0; // 右路径长度
        TreeNode* curr = root;
        TreeNode* predecessor = nullptr;
        
        while (curr != nullptr) {
            if (curr->left == nullptr) {
                // 没有左子树，直接转向右子树
                ++currentDepth;
                // 检查当前节点是否为叶子节点
                if (curr->left == nullptr && curr->right == nullptr && minDepth > currentDepth) {
                    minDepth = currentDepth;
                }
                curr = curr->right;
            }
            else {
                // 找到当前节点在中序遍历下的前驱节点
                rightPathLength = 1;
                predecessor = curr->left;
                
                // 寻找左子树的最右节点
                while (predecessor->right != nullptr && predecessor->right != curr) {
                    predecessor = predecessor->right;
                    ++rightPathLength;
                }
                
                if (predecessor->right == nullptr) {
                    // 第一次访问，建立临时链接
                    predecessor->right = curr;
                    ++currentDepth;
                    curr = curr->left;
                }
                else {
                    // 第二次访问，断开临时链接
                    predecessor->right = nullptr;
                    
                    // 检查前驱节点是否为叶子节点
                    if (predecessor->left == nullptr && minDepth > currentDepth) {
                        minDepth = currentDepth;
                    }
                    
                    // 调整深度并转向右子树
                    currentDepth -= rightPathLength;
                    curr = curr->right;
                }
            }
        }
        
        return minDepth;
    }
};
/*
 * @lc app=leetcode.cn id=111 lang=cpp
 *
 * [111] 二叉树的最小深度
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
    int minDepth(TreeNode* root) {
        if (!root) return 0;
        
        // 左子树为空，只计算右子树深度
        if (!root->left) return 1 + minDepth(root->right);
        // 右子树为空，只计算左子树深度
        if (!root->right) return 1 + minDepth(root->left);
        
        // 左右子树均存在，取较小值
        return 1 + min(minDepth(root->left), minDepth(root->right));
    }
};
// @lc code=end

