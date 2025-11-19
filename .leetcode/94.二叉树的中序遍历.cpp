/*
 * @lc app=leetcode.cn id=94 lang=cpp
 *
 * [94] 二叉树的中序遍历
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
std::vector<int> inorderTraversal(TreeNode* root) {
    std::vector<int> res;
    std::stack<TreeNode*> st;
    TreeNode* curr = root;
    
    while (curr != nullptr || !st.empty()) {
        if (curr != nullptr) {
            // 当前节点入栈，继续向左
            st.push(curr);
            curr = curr->left;
        } else {
            // 到达最左端，回溯
            curr = st.top();
            st.pop();
            res.push_back(curr->val);
            curr = curr->right;
        }
    }
    
    return res;
}

// morris 遍历，左子树经过两次，右子树经过一次，信息整合能力不强，递归则是左右根都能经过三次，整合信息能力强
// 如果是需要自底向上携带信息返回的，morris遍历不能胜任
/*
Morris遍历的理解核心
    没有左子树的节点只到达一次，有左子树的节点会到达两次
    利用左子树最右节点的右指针状态，来标记是第几次到达
    
Morris遍历的过程
1,开始时cur来到头节点，cur为空时过程停止
2,如果cur没有左孩子，cur向右移动
3,如果cur有左孩子，找到cur左子树的最右节点mostRight
    A,如果mostRight的右指针指向空，让其指向cur,然后cur向左移动
    B,如果mostRight的右指针指向cur,让其指向null,然后cur向右移动

额外空间复杂度很明显是0(1)，但是时间复杂度依然为0(N)
*/
std::vector<int> inorderTraversal(TreeNode* root) {
    std::vector<int> res;
    TreeNode* curr = root;
    
    while (curr != nullptr) {
        if (curr->left == nullptr) {
            // 如果没有左子树，访问当前节点并转向右子树
            res.push_back(curr->val);
            curr = curr->right;
        } else {
            // 找到当前节点的前驱节点（左子树的最右节点）
            TreeNode* predecessor = curr->left;
            while (predecessor->right != nullptr && predecessor->right != curr) {
                predecessor = predecessor->right;
            }
            
            if (predecessor->right == nullptr) {
                // 建立临时链接，便于回溯
                predecessor->right = curr;
                curr = curr->left;
            } else {
                // 断开临时链接，访问当前节点
                predecessor->right = nullptr;
                res.push_back(curr->val);
                curr = curr->right;
            }
        }
    }
    
    return res;
}
class Solution {
public:
    //先序：根左右
    //中序：左根右
    //后续：右左根
    vector<int> inorderTraversal(TreeNode* root) {
        vector<int> result;
        traversalHelper(root, result);
        return result;
    }
private:
    void traversalHelper(TreeNode* node, vector<int>& res) {
        if (node == nullptr) return;
        
        // ========== 遍历方式选择开关 ==========
#ifdef PRE_ORDER
        // 前序遍历：根 -> 左 -> 右
        res.push_back(node->val);           // 访问根
        traversalHelper(node->left, res);   // 递归左子树
        traversalHelper(node->right, res);  // 递归右子树

#elif defined(POST_ORDER)
        // 后序遍历：左 -> 右 -> 根
        traversalHelper(node->left, res);   // 递归左子树
        traversalHelper(node->right, res);  // 递归右子树
        res.push_back(node->val);           // 访问根

#else
        // 默认中序遍历：左 -> 根 -> 右
        traversalHelper(node->left, res);   // 递归左子树
        res.push_back(node->val);           // 访问根
        traversalHelper(node->right, res);  // 递归右子树
#endif
    }
};
// @lc code=end

