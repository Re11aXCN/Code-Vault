/*
 * @lc app=leetcode.cn id=236 lang=cpp
 *
 * [236] 二叉树的最近公共祖先
 */

// @lc code=start
/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */
class Solution {
public:
// 临时建立父路径，通过父路径找
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        if (!root) return nullptr;

        std::unordered_map<TreeNode*, TreeNode*> parentMap;
        std::stack<TreeNode*> stk;
        stk.push(root);
        parentMap[root] = nullptr;  // 根节点的父节点为nullptr
        bool pfind = false, qfind = false;
        while (!stk.empty()) {
            TreeNode* node = stk.top();
            stk.pop();

            if (node->right) {
                parentMap[node->right] = node;
                stk.push(node->right);
            }
            if (node->left) {
                parentMap[node->left] = node;
                stk.push(node->left);
            }
            if (node == p) {
                pfind = true;
                if (qfind) break;
            }
            if (node == q) {
                qfind = true;
                if (pfind) break;
            }
        }
        auto ppaths = getPathToRoot(p, parentMap);
        auto qpaths = getPathToRoot(q, parentMap);
        int ppathLength = ppaths.size(), qpathLength = qpaths.size();
        if (ppathLength < qpathLength) {
            for (auto pit = ppaths.begin(), qit = qpaths.begin() + (qpathLength - ppathLength); pit != ppaths.end() && qit != qpaths.end(); ++pit, ++qit) {
                if(*pit == *qit) return *pit;
            }
        }
        else {
            for (auto pit = ppaths.begin() + (ppathLength - qpathLength), qit = qpaths.begin(); pit != ppaths.end() && qit != qpaths.end(); ++pit, ++qit) {
                if (*pit == *qit) return *pit;
            }
        }
        return root;
    }
private:
    std::vector<TreeNode*> getPathToRoot(TreeNode* node, std::unordered_map<TreeNode*, TreeNode*> parentMap) {
        std::vector<TreeNode*> path;
        while (node) {
            path.push_back(node);
            node = parentMap[node];
        }
        return path;
    }
};

// 迭代DFS + 路径深度优化
class Solution {
public:
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        if (!root || root == p || root == q) return root;

        unordered_map<TreeNode*, TreeNode*> parentMap;
        unordered_map<TreeNode*, int> depthMap; // 记录节点深度
        stack<pair<TreeNode*, int>> stk; // 节点和当前深度

        stk.push({ root, 0 });
        parentMap[root] = nullptr;
        depthMap[root] = 0;

        int foundCount = 0;

        while (!stk.empty() && foundCount < 2) {
            auto [node, depth] = stk.top();
            stk.pop();

            if (node == p || node == q) foundCount++;

            if (node->right) {
                parentMap[node->right] = node;
                depthMap[node->right] = depth + 1;
                stk.push({ node->right, depth + 1 });
            }
            if (node->left) {
                parentMap[node->left] = node;
                depthMap[node->left] = depth + 1;
                stk.push({ node->left, depth + 1 });
            }
        }

        return findLCAOptimized(parentMap, depthMap, p, q);
    }

private:
    TreeNode* findLCAOptimized(unordered_map<TreeNode*, TreeNode*>& parentMap,
        unordered_map<TreeNode*, int>& depthMap,
        TreeNode* p, TreeNode* q) {
        // 将较深的节点提升到同一深度
        TreeNode* node1 = p, * node2 = q;
        int depth1 = depthMap[p], depth2 = depthMap[q];

        while (depth1 > depth2) {
            node1 = parentMap[node1];
            depth1--;
        }
        while (depth2 > depth1) {
            node2 = parentMap[node2];
            depth2--;
        }

        // 同时向上回溯直到找到公共祖先
        while (node1 != node2) {
            node1 = parentMap[node1];
            node2 = parentMap[node2];
        }

        return node1;
    }
};

// 最简迭代版本（空间最优） 性能比上述两个好
class Solution {
public:
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        if (!root || root == p || root == q) return root;
        
        // 单次遍历，使用parent指针和深度信息
        TreeNode* lca = nullptr;
        unordered_map<TreeNode*, TreeNode*> parent;
        unordered_map<TreeNode*, int> depth;
        stack<TreeNode*> stk;
        
        stk.push(root);
        parent[root] = nullptr;
        depth[root] = 0;
        
        while (!stk.empty()) {
            TreeNode* node = stk.top();
            stk.pop();
            
            // 如果已经找到两个节点，可以提前终止
            if (parent.contais(p) && parent.contais(q)) {
                break;
            }
            
            if (node->right) {
                parent[node->right] = node;
                depth[node->right] = depth[node] + 1;
                stk.push(node->right);
            }
            if (node->left) {
                parent[node->left] = node;
                depth[node->left] = depth[node] + 1;
                stk.push(node->left);
            }
        }
        
        // 对齐深度后向上查找
        TreeNode* a = p, *b = q;
        while (depth[a] > depth[b]) a = parent[a];
        while (depth[b] > depth[a]) b = parent[b];
        while (a != b) {
            a = parent[a];
            b = parent[b];
        }
        
        return a;
    }
};

// 进一步优化空间
class Solution {
public:
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        if (!root || root == p || root == q) return root;
        
        stack<TreeNode*> stk;
        unordered_map<TreeNode*, TreeNode*> parent;
        unordered_set<TreeNode*> visited;
        
        stk.push(root);
        parent[root] = nullptr;
        
        // 迭代直到找到p和q的父节点关系
        while (!parent.count(p) || !parent.count(q)) {
            TreeNode* node = stk.top();
            stk.pop();
            
            if (node->right) {
                parent[node->right] = node;
                stk.push(node->right);
            }
            if (node->left) {
                parent[node->left] = node;
                stk.push(node->left);
            }
        }
        
        // 从p开始向上标记路径
        TreeNode* current = p;
        while (current) {
            visited.insert(current);
            current = parent[current];
        }
        
        // 从q开始向上找第一个被标记的节点
        current = q;
        while (!visited.count(current)) {
            current = parent[current];
        }
        
        return current;
    }
};

class Solution {
public:
    // 利用递归回溯的特性能够携带节点信息，携带p、q节点（左右信息）信息往上传递（给根），后续遍历非常适合

    // 以下是原理
    // 1、分别在不同的左右子树 满足left && right
    // 2、在左子树中有p、q节点，找到其中一个直接返回（因为1条件可以判断另一个节点的信息），满足left；右子树同理，满足right
    // 
    // 例如：分别在不同的左右子树
    // 给定如下二叉树：找4、0的最近公共祖先节点
    //     3             3  没找到返回null     3      最终如   3    传递4、0，满足left && right，返回3
    //    / \           / \    否则传递pq     / \            / \
    //   5   1         5   1                5   1          4   0
    //  / \ / \       / \ / \              / \ / \        / \ / \
    // 6  2 0  8     n  2 0  8            n  4 0  8      n  4 0  n
    //   / \           / \                  / \            / \
    //  7   4         n   4                n   4          n   4
    // 
    // 例如：在左子树中有p、q节点
    // 给定如下二叉树：找5、4的最近公共祖先节点
    //     3    
    //    / \   找到5之直接返回，遍历右子树没找到q = 4，说明q必定在p=5的树下，全部返回null，结束满足left，return 5
    //   5   n  
    //  / \ / \ 
    // 6  2 n  n
    //   / \    
    //  7   4   
    // 总结：找到p、q就携带p、q信息，没找到就携带null信息，最终根据携带信息是否不是null判断找到最近公共祖先节点到底是谁

    //适用于所有类型的二叉树，不仅限于二叉搜索树。它通过后序遍历的方式，自底向上地查找最近公共祖先，是解决这类问题的通用方法。
    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        // 如果根节点为空，或者根节点就是p或q中的一个，直接返回根节点
        if (!root || root == p || root == q) {
            return root;
        }

        // 递归搜索左子树
        TreeNode* left = lowestCommonAncestor(root->left, p, q);
        // 递归搜索右子树
        TreeNode* right = lowestCommonAncestor(root->right, p, q);

        // 如果左子树和右子树都找到了结果，说明p和q分别在当前节点的左右子树中
        // 当前节点就是最近公共祖先
        if (left && right)  return root;

        // 如果只有左子树找到了结果，返回左子树的结果
        if (left) return left;

        // 如果只有右子树找到了结果，返回右子树的结果
        return right;
    }
};
// @lc code=end

