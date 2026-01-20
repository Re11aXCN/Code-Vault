    void reorderList(ListNode* head) {
        ListNode* slow = head, * fast = head->next;
        while (fast && fast->next) {
            fast = fast->next->next;
            slow = slow->next;
        }
        fast = slow->next;
        slow->next = nullptr;
        slow = head;
        ListNode dummy(0, nullptr);
        while (fast) {
            ListNode* temp = fast->next;
            fast->next = dummy.next;
            dummy.next = fast;
            fast = temp;
        }
        fast = &dummy;
        while (fast->next) {
            ListNode* temp = slow->next;
            slow->next = fast->next;
            fast->next = slow;
            fast = slow->next;
            slow = temp;
        }
        fast->next = slow;
    }
    void reorderList(ListNode* head) {
        ListNode* fast{head}, * slow{head}, *slowPrev{nullptr};
        while (fast && fast->next) {
            fast = fast->next->next;
            slowPrev = slow;
            slow = slow->next;
        }
        ListNode dummy{0, nullptr};
        fast = slow;
        slowPrev->next = nullptr;
        while (fast) {
            ListNode* fastNext = fast->next;
            fast->next = dummy.next;
            dummy.next = fast;
            fast = fastNext;
        }
        slow = head, fast = dummy.next;
        while (slow) {
            if (!slow->next) [[unlikely]] slowPrev = fast;
            ListNode* fastNext = fast->next;
            fast->next = slow->next;
            slow->next = fast;
            slow = fast->next;
            fast = fastNext;
        }
        slowPrev->next = fast;
    }
/*
 * @lc app=leetcode.cn id=143 lang=cpp
 *
 * [143] 重排链表
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
    void reorderList(ListNode* head) {
#define SLOW_FAST_PTR
#ifndef SLOW_FAST_PTR
        std::vector<ListNode*> parent;
        ListNode* ptr = head;
        ListNode* temp = nullptr;
        while (ptr)
        {
            parent.push_back(ptr);
            temp = ptr;
            ptr = ptr->next;
            temp->next = nullptr;
        }
        for (auto left = parent.begin(), right = parent.end() - 1;
            left < right; ++left, --right)
        {
            (*left)->next = *right;
            (*right)->next = left + 1 == right ? nullptr : *(left + 1);
        }
#else
        if (!head || !head->next) return;

        // 1. 使用快慢指针找到链表中间节点
        ListNode *slow = head, *fast = head;
        while (fast->next && fast->next->next) {
            slow = slow->next;
            fast = fast->next->next;
        }
        // 2. 反转后半部分链表
        ListNode *prev = nullptr, *curr = slow->next;
        slow->next = nullptr; // 断开前后两部分
        
        while (curr) {
            ListNode *nextTemp = curr->next;
            curr->next = prev;
            prev = curr;
            curr = nextTemp;
        }
        // 3. 合并两个链表
        ListNode *first = head, *second = prev;
        while (second) {
            ListNode *temp1 = first->next;
            ListNode *temp2 = second->next;
            
            first->next = second;
            second->next = temp1;
            
            first = temp1;
            second = temp2;
        }
#endif
    }
};
// @lc code=end

