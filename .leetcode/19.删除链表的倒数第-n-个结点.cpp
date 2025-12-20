/*
 * @lc app=leetcode.cn id=19 lang=cpp
 *
 * [19] 删除链表的倒数第 N 个结点
 */

// @lc code=start
/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode() : val(0), next(nullptr) {}
 *     ListNode(int x) : val(x), next(nullptr) {}
 *     ListNode(int x, ListNode *next) : val(x), next(next) {}
 * };
 */
#include <vector>
using namespace std;

class Solution {
public:
    ListNode* removeNthFromEnd(ListNode* head, int n) {
        // ================= 解法选择开关 =================
//#define TWO_PASS        // 两次遍历法，时间O(L) 空间O(1)
//#define ARRAY_STORAGE  // 数组存储法，时间O(L) 空间O(L)
#define FAST_SLOW_PTR  // 快慢指针法，时间O(L) 空间O(1)

#ifdef TWO_PASS
        // 方法一：两次遍历法
        // 1. 第一次遍历计算链表长度
        int len = 0;
        ListNode* current = head;
        while (current) {
            ++len;
            current = current->next;
        }
        
        // 计算要删除节点的正数位置（从0开始）
        int pos = len - n;
        if (pos == 0) { // 删除头节点
            ListNode* newHead = head->next;
            delete head; // 根据题目要求是否实际删除节点
            return newHead;
        }
        
        // 2. 第二次遍历到目标节点的前驱
        current = head;
        for (int i = 0; i < pos - 1; ++i) {
            current = current->next;
        }
        ListNode* toDelete = current->next;
        current->next = toDelete->next;
        delete toDelete; // 根据题目要求是否实际删除节点
        return head;

#elif defined(ARRAY_STORAGE)
        // 方法二：数组存储法
        std::vector<ListNode*> nodes;
        nodes.reserve(15);
        while (head) nodes.push_back(head), head = head->next;
        auto deleteNodePos = nodes.end() - n;
        if (*deleteNodePos == nodes.front()) {
            delete* deleteNodePos, *deleteNodePos = nullptr;
            return nodes.size() == 1 ? nullptr : nodes[1];
        }
        (*(deleteNodePos - 1))->next = *deleteNodePos == nodes.back() ? nullptr : *(deleteNodePos + 1);
        delete* deleteNodePos, *deleteNodePos = nullptr;
        return nodes.front();

#else
        // 方法三：快慢指针法（最优解）
        ListNode dummy(0, head); // 处理单个节点删除为空情况
        ListNode* slow = &dummy;
        ListNode* fast = &dummy;

        
        // 快指针先移动n+1步，使快慢指针间隔n个节点
        // +1是让slow得到删除节点前一个节点
        for (int i = 0; i < n + 1; ++i) {
            if (!fast) break; // 处理n超过链表长度的情况
            fast = fast->next;
        }
        
        // 同步移动直到快指针到末尾
        while(fast) fast = fast->next, slow = slow->next; 
        
        // 此时slow指向待删除节点的前驱
        ListNode* deleteNode = slow->next;
        slow->next = deleteNode->next;
        delete deleteNode, deleteNode = nullptr; // 根据题目要求是否实际删除节点
        return dummy.next;
#endif
    }
};
// @lc code=end

