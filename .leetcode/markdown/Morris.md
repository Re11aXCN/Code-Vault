# Morris代码

```
TreeNode* curr = root;
while(curr) {
    TreeNode* predecessor = curr->left;
    if (predecessor) {
        while (predecessor->right && predecessor->right != curr) predecessor = predecessor->right;

        if (predecessor->right) {
            // 情况3：线索已经建立，说明左子树已经遍历完
            // 这里应该访问当前节点（收集数据），然后断开线索，转向右子树
            // 但是代码中并没有断开线索，而是直接转向右子树，这可能会造成问题。
            // 而且，这里没有收集数据。
            prev = curr;  // 这里似乎是在记录前一个节点，但并没有收集当前节点的数据
            curr = curr->right;
        }
        else {
            // 情况2：建立线索，然后转向左子树
            predecessor->right = curr;
            curr = curr->left;
        }
    }
    else {
        // 情况1：没有左子树
        // 这里应该访问当前节点（收集数据），然后转向右子树
        // 但是代码中并没有收集数据，而是直接转向右子树
        curr = curr->right;
    }
}

```

## 特性总结表格：

| 情况 | 描述       | 中序遍历收集位置 | 前序遍历收集位置 |
| ---- | ---------- | ---------------- | ---------------- |
| 1    | 没有左子树 | ✅ 此处收集       | ✅ 此处收集       |
| 2    | 建立线索   | ❌ 不收集         | ✅ 此处收集       |
| 3    | 线索已建立 | ✅ 此处收集       | ❌ 不收集         |

注意：后序遍历的实现会更复杂，通常需要额外的反转操作或标记。



| 节点类型 | 访问次数 | 第一次访问时操作   | 第二次访问时操作   |
| -------- | -------- | ------------------ | ------------------ |
| 有左子树 | 2次      | 建立线索，向左移动 | 断开线索，向右移动 |
| 无左子树 | 1次      | 直接向右移动       | 无第二次访问       |

**关键点**：Morris遍历通过巧妙的线索化，让有左子树的节点能够被"返回"访问第二次，从而在不使用栈的情况下完成遍历。这种设计使得空间复杂度为O(1)，同时保持了O(n)的时间复杂度。

Morris遍历虽然空间效率高，但在某些场景下不适用或不理想。以下是主要的不适用场景：

## 后序遍历

### 思路（边收集反转）

```C++
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

// Morris后序遍历主函数
vector<int> postorderTraversal(TreeNode* root) {
    vector<int> result;
    if (!root) return result;
    
    TreeNode *dummy = new TreeNode(0);  // 创建虚拟节点
    dummy->left = root;
    TreeNode *curr = dummy;
    
    while (curr) {
        if (curr->left) {
            // 找到当前节点的前驱节点（左子树的最右节点）
            TreeNode *predecessor = curr->left;
            while (predecessor->right && predecessor->right != curr) {
                predecessor = predecessor->right;
            }
            
            if (!predecessor->right) {
                // 建立线索
                predecessor->right = curr;
                curr = curr->left;
            } else {
                // 线索已建立，断开线索并收集节点
                predecessor->right = nullptr;
                // 收集从当前节点的左孩子到前驱节点路径上的所有节点
                collectNodes(curr->left, predecessor, result);
                curr = curr->right;
            }
        } else {
            curr = curr->right;
        }
    }
    
    delete dummy;
    return result;
}

// 辅助函数：收集从from到to路径上的节点（逆序）
void collectNodes(TreeNode* from, TreeNode* to, vector<int>& result) {
    // 反转从from到to的路径
    reversePath(from, to);
    
    // 收集反转后的节点
    TreeNode *node = to;
    while (node) {
        result.push_back(node->val);
        if (node == from) break;
        node = node->right;
    }
    
    // 恢复原始路径
    reversePath(to, from);
}

// 辅助函数：反转从from到to的路径（通过right指针连接）
void reversePath(TreeNode* from, TreeNode* to) {
    if (from == to) return;
    
    TreeNode *prev = from;
    TreeNode *curr = from->right;
    TreeNode *next = nullptr;
    
    while (prev != to) {
        next = curr->right;
        curr->right = prev;
        prev = curr;
        curr = next;
    }
}
```



### 思路（先得到根-右-左，收集完成，最后反转）

```c++
vector<int> morrisPostorderSimple(TreeNode* root) {
    vector<int> result;
    TreeNode *curr = root;
    
    // 第一步：使用Morris遍历得到"根->右->左"的顺序
    while (curr) {
        if (curr->right) {
            TreeNode *predecessor = curr->right;
            while (predecessor->left && predecessor->left != curr) {
                predecessor = predecessor->left;
            }
            
            if (!predecessor->left) {
                // 第一次访问curr节点
                result.push_back(curr->val);  // 收集根节点
                predecessor->left = curr;     // 建立线索
                curr = curr->right;           // 向右移动
            } else {
                // 第二次访问，断开线索
                predecessor->left = nullptr;
                curr = curr->left;
            }
        } else {
            // 没有右子树
            result.push_back(curr->val);
            curr = curr->left;
        }
    }
    
    // 第二步：反转结果得到"左->右->根"
    reverse(result.begin(), result.end());
    return result;
}
```



# 不适用场景

## 1. **需要保持树结构不变/并发访问的场景**

- **问题**：Morris遍历会临时修改树的指针（建立线索）
- **场景**：
  - 多线程环境下，其他线程同时访问树
  - 需要保持树结构只读的场合
  - 数据库索引等不允许修改的结构
  - 实时系统需要保证数据一致性

cpp

```
// Morris遍历会修改指针
predecessor->right = curr;  // 这改变了树结构
```



## 2. **需要重复遍历的场景**

- **问题**：每次遍历都需要重新建立和断开线索
- **场景**：
  - 需要多次遍历同一棵树
  - 需要同时进行前序、中序、后序遍历
  - 遍历过程中需要暂停和恢复

## 3. **内存访问限制严格的场景**

- **问题**：频繁修改指针可能导致缓存失效
- **场景**：
  - 高性能计算，需要缓存友好
  - 嵌入式系统，内存访问受限
  - 实时系统，需要可预测的性能

## 4. **需要快速失败/错误检测的场景**

- **问题**：异常情况可能破坏树结构

- **场景**：

  cpp

```
// 如果这里抛出异常，线索可能没有被正确断开
if (predecessor->right) {
    // 异常发生在这里...
    throw std::runtime_error("Error");
    predecessor->right = nullptr;  // 可能不会执行
}
```



## 5. **需要遍历过程中回溯到父节点的场景**

- **问题**：Morris遍历通过线索回溯，但回溯路径有限
- **场景**：
  - 需要频繁访问父节点
  - 需要从任意节点向上遍历
  - 需要父子节点同时处理

## 6. **树节点有特殊约束的场景**

- **问题**：某些树的实现不允许修改指针
- **场景**：
  - 共享内存中的树结构
  - 持久化数据结构
  - 函数式编程中的不可变树

## 7. **调试和维护困难**

- **问题**：代码复杂，难以理解和维护

- **场景**：

  cpp

```
// 传统的栈遍历更容易理解
void inorder(TreeNode* root) {
    stack<TreeNode*> st;
    while (root || !st.empty()) {
        while (root) {
            st.push(root);
            root = root->left;
        }
        root = st.top(); st.pop();
        cout << root->val << " ";
        root = root->right;
    }
}
```



## 8. **特定类型的树**

- **问题**：不适用于所有树变体
- **场景**：
  - **线索二叉树**：已经包含线索，Morris会破坏
  - **平衡树（AVL、红黑树）**：修改指针可能影响平衡
  - **B树/B+树**：节点结构不同

## 9. **需要稳定时间复杂度的场景**

- **问题**：虽然平均O(n)，但常数因子较大
- **场景**：
  - 实时控制系统
  - 硬实时系统
  - 对性能波动敏感的应用

## 10. **需要后序遍历的复杂场景**

- **问题**：Morris后序遍历实现复杂

- **场景**：

  cpp

```
// Morris后序遍历实现复杂，需要反转链表
void morrisPostorder(TreeNode* root) {
    // 实现复杂，容易出错
    // 需要额外的反转操作
}
```



## 对比表格：

| 场景         | 传统栈遍历 | Morris遍历 | 推荐选择 |
| ------------ | ---------- | ---------- | -------- |
| 多线程访问   | ✅ 安全     | ❌ 不安全   | 栈遍历   |
| 需要重复遍历 | ✅ 高效     | ❌ 低效     | 栈遍历   |
| 内存充足     | ✅ 适合     | ⚠️ 可考虑   | 栈遍历   |
| 内存受限     | ⚠️ 可能溢出 | ✅ 适合     | Morris   |
| 代码可读性   | ✅ 简单     | ❌ 复杂     | 栈遍历   |
| 实时系统     | ✅ 稳定     | ❌ 不稳定   | 栈遍历   |
| 嵌入式系统   | ❌ 内存大   | ✅ 内存小   | Morris   |

## 实际选择建议：

1. **使用传统栈遍历的情况**：

   - 开发速度快，代码可读性重要
   - 内存充足，空间复杂度不是瓶颈
   - 需要线程安全
   - 需要多次遍历同一棵树

2. **使用Morris遍历的情况**：

   - 内存极度受限（嵌入式设备）
   - 明确要求O(1)空间复杂度
   - 只需单次遍历，且不需保持树结构
   - 作为算法优化技巧展示

3. **折中方案**：

   cpp

```
// 对于实际项目，考虑封装和抽象
class TreeIterator {
private:
    // 根据场景选择实现
    // 内存紧张：使用Morris
    // 需要安全：使用栈
    // 提供统一接口
};
```



**总结**：Morris遍历是一种聪明的算法技巧，但在实际工程中需要谨慎使用，特别是在需要稳定性、可维护性和线程安全的场景。