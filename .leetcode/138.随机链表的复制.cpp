/*
 * @lc app=leetcode.cn id=138 lang=cpp
 *
 * [138] 随机链表的复制
 */

// @lc code=start
/*
// Definition for a Node.
class Node {
public:
    int val;
    Node* next;
    Node* random;
    
    Node(int _val) {
        val = _val;
        next = NULL;
        random = NULL;
    }
};
*/

class Solution {
public:
    Node* copyRandomList(Node* head) {
                // ================ 解法选择开关 ================
//#define HASH_MAP_METHOD   // 哈希表法 O(n)空间
#define IN_PLACE_METHOD  // 原地复制法 O(1)空间

#ifdef HASH_MAP_METHOD
        /* 方法一：哈希表映射法（直观易懂）*/
        if (!head) return nullptr;

        // 1. 建立原节点到拷贝节点的映射
        unordered_map<Node*, Node*> nodeMap;
        Node* curr = head;
        while (curr) {
            nodeMap[curr] = new Node(curr->val); // 仅拷贝值
            curr = curr->next;
        }

        // 2. 设置拷贝节点的next和random指针
        curr = head;
        while (curr) {
            Node* copyNode = nodeMap[curr];
            copyNode->next = nodeMap[curr->next];  // 映射next关系
            copyNode->random = nodeMap[curr->random]; // 映射random关系
            curr = curr->next;
        }
        
        return nodeMap[head]; // 返回拷贝链表的头节点

#else
        /* 方法二：原地复制法（空间最优）*/
        if (!head) return nullptr;

        // 1. 插入克隆节点：A → A' → B → B' → ...
        Node* curr = head;
        while (curr) {
            Node* clone = new Node(curr->val);
            clone->next = curr->next;
            curr->next = clone;
            curr = clone->next; // 移动到原链表的下一个节点
        }

        // 2. 设置克隆节点的random指针
        curr = head;
        while (curr) {
            if (curr->random) {
                // 克隆节点的random = 原节点random的克隆（即原random的next）
                curr->next->random = curr->random->next;
            }
            curr = curr->next->next; // 跳过克隆节点
        }

        // 3. 分离原始链表和克隆链表
        Node dummy(0);      // 哑节点简化操作
        Node* copyCurr = &dummy;
        curr = head;
        while (curr) {
            copyCurr->next = curr->next;    // 连接克隆节点
            copyCurr = copyCurr->next;      // 移动克隆链表指针
            
            curr->next = curr->next->next;  // 恢复原始链表结构
            curr = curr->next;              // 移动原始链表指针
        }
        
        return dummy.next;
#endif
    }
};
// @lc code=end

