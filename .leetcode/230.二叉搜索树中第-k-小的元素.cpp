/*
 * @lc app=leetcode.cn id=230 lang=cpp
 *
 * [230] 二叉搜索树中第 K 小的元素
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
    int kthSmallest(TreeNode* root, int k) {
        // 记录当前遍历到第几个节点
        int count = 0;
        // 记录结果
        int result = 0;
        
        // 中序遍历
        inorder(root, k, count, result);
        
        return result;
    }
    
private:
    // 中序遍历函数
    void inorder(TreeNode* node, int k, int& count, int& result) {
        // 如果节点为空或已经找到第k小的元素，直接返回
        if (node == nullptr || count >= k) {
            return;
        }
        
        // 先遍历左子树
        inorder(node->left, k, count, result);
        
        // 访问当前节点，计数加1
        count++;
        // 如果是第k个节点，记录结果
        if (count == k) {
            result = node->val;
            return;
        }
        
        // 再遍历右子树
        inorder(node->right, k, count, result);
    }
};
// @lc code=end

/**
 * 带计数的二叉搜索树节点定义
 */
struct TreeNodeWithCount {
    int val;           // 节点值
    int leftCount;     // 左子树节点数量
    TreeNodeWithCount* left;
    TreeNodeWithCount* right;
    
    // 构造函数
    TreeNodeWithCount(int x) : val(x), leftCount(0), left(nullptr), right(nullptr) {}
};

/**
 * 优化的二叉搜索树类，支持高效查找第k小元素和修改操作
 */
class OptimizedBST {
private:
    TreeNodeWithCount* root;
    
    // 查找第k小的元素的辅助函数
    int kthSmallestHelper(TreeNodeWithCount* node, int k) {
        if (!node) return -1; // 树为空或k超出范围
        
        int leftSize = node->leftCount;
        
        if (k <= leftSize) {
            // 第k小的元素在左子树中
            return kthSmallestHelper(node->left, k);
        } else if (k == leftSize + 1) {
            // 当前节点就是第k小的元素
            return node->val;
        } else {
            // 第k小的元素在右子树中，需要减去左子树的大小和当前节点
            return kthSmallestHelper(node->right, k - leftSize - 1);
        }
    }
    
    // 插入节点的辅助函数
    TreeNodeWithCount* insertHelper(TreeNodeWithCount* node, int val) {
        if (!node) {
            return new TreeNodeWithCount(val);
        }
        
        if (val < node->val) {
            // 在左子树中插入，左子树节点数加1
            node->left = insertHelper(node->left, val);
            node->leftCount++;
        } else {
            // 在右子树中插入
            node->right = insertHelper(node->right, val);
        }
        
        return node;
    }
    
    // 查找最小节点
    TreeNodeWithCount* findMin(TreeNodeWithCount* node) {
        while (node->left) {
            node = node->left;
        }
        return node;
    }
    
    // 删除节点的辅助函数
    TreeNodeWithCount* deleteHelper(TreeNodeWithCount* node, int val) {
        if (!node) return nullptr;
        
        if (val < node->val) {
            // 在左子树中删除，左子树节点数减1
            node->left = deleteHelper(node->left, val);
            node->leftCount--;
        } else if (val > node->val) {
            // 在右子树中删除
            node->right = deleteHelper(node->right, val);
        } else {
            // 找到要删除的节点
            
            // 情况1: 叶子节点
            if (!node->left && !node->right) {
                delete node;
                return nullptr;
            }
            // 情况2: 只有一个子节点
            else if (!node->left) {
                TreeNodeWithCount* temp = node->right;
                delete node;
                return temp;
            }
            else if (!node->right) {
                TreeNodeWithCount* temp = node->left;
                delete node;
                return temp;
            }
            // 情况3: 有两个子节点
            else {
                // 找到右子树的最小节点（后继节点）
                TreeNodeWithCount* successor = findMin(node->right);
                // 复制后继节点的值
                node->val = successor->val;
                // 删除后继节点
                node->right = deleteHelper(node->right, successor->val);
            }
        }
        
        return node;
    }
    
    // 清理内存的辅助函数
    void clearHelper(TreeNodeWithCount* node) {
        if (!node) return;
        
        clearHelper(node->left);
        clearHelper(node->right);
        delete node;
    }
    
public:
    // 构造函数
    OptimizedBST() : root(nullptr) {}
    
    // 析构函数
    ~OptimizedBST() {
        clear();
    }
    
    // 插入元素
    void insert(int val) {
        root = insertHelper(root, val);
    }
    
    // 删除元素
    void remove(int val) {
        root = deleteHelper(root, val);
    }
    
    // 查找第k小的元素
    int kthSmallest(int k) {
        return kthSmallestHelper(root, k);
    }
    
    // 清理整棵树
    void clear() {
        clearHelper(root);
        root = nullptr;
    }
};

// 使用示例
int main() {
    OptimizedBST bst;
    
    // 插入元素
    bst.insert(5);
    bst.insert(3);
    bst.insert(7);
    bst.insert(2);
    bst.insert(4);
    bst.insert(6);
    bst.insert(8);
    
    // 查找第k小的元素
    std::cout << "第1小的元素: " << bst.kthSmallest(1) << std::endl; // 应该输出2
    std::cout << "第3小的元素: " << bst.kthSmallest(3) << std::endl; // 应该输出4
    std::cout << "第5小的元素: " << bst.kthSmallest(5) << std::endl; // 应该输出6
    
    // 删除元素
    bst.remove(3);
    
    // 再次查找
    std::cout << "删除3后，第2小的元素: " << bst.kthSmallest(2) << std::endl; // 应该输出4
    
    return 0;
}
