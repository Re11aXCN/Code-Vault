bool isPalindrome(ListNode* head) {
    if (!head || !head->next) [[unlikely]] return true;
    ListNode *fast{head->next}, *slow{head}, *tail{nullptr};
    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    tail = slow;
    fast = tail->next;
    tail->next = nullptr;
    slow = head;

    ListNode dummy{0, nullptr};
    while (fast) {
        ListNode* fastNext = fast->next;
        fast->next = dummy.next;
        dummy.next = fast;
        fast = fastNext;
    }

    fast = dummy.next;
    while (fast) {
        if (fast->val != slow->val) break;
        fast = fast->next;
        slow = slow->next;
    }

    slow = dummy.next;
    while (slow) {
        ListNode* slowNext = slow->next;
        slow->next = tail->next;
        tail->next = slow;
        slow = slowNext;
    }
    return fast == nullptr;
};
/*
 * @lc app=leetcode.cn id=234 lang=cpp
 *
 * [234] 回文链表
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
    bool isPalindrome(ListNode* head) {
        if (!head || !head->next) return true;

        // 步骤1：使用快慢指针找到链表中点
        ListNode* slow = head;
        ListNode* fast = head;
        while (fast->next && fast->next->next) {
            slow = slow->next;       // 慢指针每次移动一步
            fast = fast->next->next; // 快指针每次移动两步
        }

        // 步骤2：反转后半部分链表
        ListNode* reversedSecondHalf = reverseList(slow->next);

        // 步骤3：比较前半部分和反转后的后半部分
        ListNode* p1 = head;
        ListNode* p2 = reversedSecondHalf;
        while (p2) { // 只需遍历后半部分长度
            if (p1->val != p2->val)  return false;
            p1 = p1->next;
            p2 = p2->next;
        }

        // 步骤4：恢复链表结构（可选，题目未要求）
        //slow->next = reverseList(reversedSecondHalf);

        return true;
    }

private:
    // 辅助函数：反转链表并返回新头节点
    ListNode* reverseList(ListNode* head) {
        ListNode* prev = nullptr;
        ListNode* curr = head;
        while (curr) {
            ListNode* nextNode = curr->next;
            curr->next = prev;
            prev = curr;
            curr = nextNode;
        }
        return prev;
    }
    /*
    bool isPalindrome(ListNode* head) {
        if (!head->next) return true;
        int length = 0;
        ListNode* current = head;
        while (current) {
            current = current->next;
            ++length;
        }
        int count = 0;
        ListNode* prev = nullptr;
        ListNode* back = nullptr;
        current = head;
        while (current) {
            ListNode* nextNode = current->next;  // 保存下一节点
            current->next = prev;                // 反转指针
            prev = current;                      // 前移prev
            current = nextNode;                  // 前移current
            if (++count == length / 2) break; 
        }
        count = 0;
        back = length % 2 == 0 ? current : current->next;
        while(count != length / 2) {
            if(prev->val != back->val) return false;
            prev = prev->next;
            back = back->next;
            ++count;
        }
        return true;
    }
    */
};
// @lc code=end
int main() {
    Solution s;
    ListNode* head = new ListNode(1);
    head->next = new ListNode(2);
    head->next->next = new ListNode(3);
    head->next->next->next = new ListNode(2);
    head->next->next->next->next = new ListNode(1);
    s.isPalindrome(head);
    return 0;
}

