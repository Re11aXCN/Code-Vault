ListNode* reverseBetween(ListNode* head, int left, int right) {
    ListNode dummy1(0, nullptr), dummy2(0, nullptr), * front = &dummy1;
    front->next = head;
    int idx = 0;
    while(++idx != left) {
        front = front->next;
    }
    ListNode* back = front->next;
    while(idx++ <= right) {
        ListNode* temp = back->next;
        back->next = dummy2.next;
        dummy2.next = back;
        back = temp;
    }
    front->next->next = back;
    front->next = dummy2.next;
    return dummy1.next;
}
/*
 * @lc app=leetcode.cn id=92 lang=cpp
 *
 * [92] 反转链表 II
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
class Solution {
public:
// 一个链表头插法反转，另一个是没有反转的链表，最后拼接
    ListNode* reverseBetween(ListNode* head, int left, int right) {
        ListNode* originPrev = nullptr;
        ListNode* reverseTail = nullptr;
        ListNode* curr = head;
        ListNode* reverseNode = new ListNode();
        ListNode* originNode = new ListNode();
        originPrev = originNode;
        originNode->next = head;

        int idx = 1;
        bool find = false;
        while (idx != right + 1)
        {
            if (find) {
                originPrev->next = curr->next;
                curr->next = reverseNode->next;
                reverseNode->next = curr;
                curr = originPrev->next;
            }
            if (idx == left) {
                find = true;
                reverseNode->next = curr;
                originPrev->next = curr->next;
                curr = curr->next;
                reverseNode->next->next = nullptr;
                reverseTail = reverseNode->next;
            }

            if (!find) {
                originPrev = curr;
                curr = curr->next;
            }
            ++idx;
        }
        reverseTail->next = originPrev->next;
        originPrev->next = reverseNode->next;
        ListNode* res = originNode->next;
        delete originNode;
        delete reverseNode;
        return res;
    }
};

/**
 * 反转链表中从位置 left 到 right 的节点（包含两端）
 * 思路：使用虚拟头节点简化边界处理，找到反转区间的前后节点，然后反转区间内的链表
 * 
 * 找到前驱left、根据[left, right]区间进行反转链表，和206 一样的逻辑，最后完成链接
 */
ListNode* reverseBetween(ListNode* head, int left, int right) {
    // 创建虚拟头节点，简化头节点处理
    ListNode* dummy = new ListNode(0);
    dummy->next = head;
    
    // Step 1: 找到反转区间的前驱节点（left-1位置）
    ListNode* prev = dummy;
    for (int i = 1; i < left; i++) {
        prev = prev->next;
    }
    
    // Step 2: 定位反转区间的起始节点和当前节点
    ListNode* reverseStart = prev->next;  // 反转区间的第一个节点
    ListNode* curr = reverseStart;
    
    // Step 3: 反转区间内的链表
    ListNode* reversePrev = nullptr;  // 用于反转的临时前驱节点
    for (int i = left; i <= right; i++) {
        ListNode* nextTemp = curr->next;  // 保存下一个节点
        curr->next = reversePrev;         // 反转指针方向
        reversePrev = curr;               // 移动前驱指针
        curr = nextTemp;                  // 移动当前指针
    }
    
    // Step 4: 重新连接链表
    // reversePrev 现在是反转后的新头节点
    // curr 现在是反转区间后的第一个节点
    prev->next = reversePrev;          // 前驱节点连接反转后的新头
    reverseStart->next = curr;         // 反转后的尾节点连接剩余部分
    
    ListNode* result = dummy->next;
    delete dummy;  // 释放虚拟头节点
    return result;
}
// @lc code=end

