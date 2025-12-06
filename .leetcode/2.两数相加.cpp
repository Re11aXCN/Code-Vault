// 第三次写
class Solution {
public:
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        if (!l1) return copyList(l2);
        if (!l2) return copyList(l1);
        ListNode dummy(0, nullptr), *p = &dummy, * p1 = l1, *p2 = l2;
        bool carry = false;
        while(p1 && p2) {
            int val = p1->val + p2->val + (carry ? 1 : 0);
            carry = val > 9;
            p->next = new ListNode(val - (carry ? 10 : 0), nullptr);
            p = p->next;
            p1 = p1->next;
            p2 = p2->next;
        }
        _constructRemain(p, p1 ? p1 : p2, carry);

        return dummy.next;
    }

    ListNode* copyList(ListNode* l) {
        ListNode dummy(0, nullptr), *p = &dummy;
        while(l) {
            p->next = new ListNode(l->val, nullptr);
            l = l->next;
        }
        return dummy.next;
    }
private:
    void _constructRemain(ListNode* head, ListNode* remain, bool& carry) {
        while(remain) {
            int val = remain->val + (carry ? 1 : 0);
            carry = val > 9;
            head->next = new ListNode(val - (carry ? 10 : 0), nullptr);
            head = head->next;
            remain = remain->next;
        }
        if (carry) head->next = new ListNode(1, nullptr);
    }

};
// 第二次写
class Solution {
public:
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        if(!l1) return l2;
        if(!l2) return l1;

        ListNode* list = new ListNode(-1, nullptr);
        ListNode* curr = list;
        ListNode* p1 = l1; 
        ListNode* p2 = l2;
        bool carry{false};
        while(p1 && p2)
        {
            int val = p1->val + p2->val;
            if(carry) ++val;
            carry = val >= 10;
            ListNode* node = new ListNode(carry ? val % 10 : val);
            curr->next = node;
            curr = curr->next;
            p1 = p1->next;
            p2 = p2->next;
        }

        if(p2) std::swap(p1, p2);
        while(p1)
        {
            int val = p1->val;
            if(carry) ++val;
            carry = val >= 10;
            curr->next = new ListNode(carry ? val % 10 : val, nullptr);
            curr = curr->next;
            p1 = p1->next;
        }
        if(carry) curr->next = new ListNode(1, nullptr);
        ListNode* res = list->next;
        delete list;
        return res;
    }
};

/*
 * @lc app=leetcode.cn id=2 lang=cpp
 *
 * [2] 两数相加
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
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        ListNode dummy(0); // 哑节点简化链表头处理
        ListNode* current = &dummy;
        int carry = 0; // 进位值，初始为0

        // 只要任一链表未遍历完或存在进位，继续循环
        while (l1 || l2 || carry) {
            // 获取当前位的值，若链表已空则补0
            int val1 = l1 ? l1->val : 0;
            int val2 = l2 ? l2->val : 0;

            int sum = val1 + val2 + carry;
            carry = sum / 10; // 计算新的进位
            current->next = new ListNode(sum % 10); // 创建新节点存储当前位结果
            current = current->next;

            // 移动链表指针（如果未到末尾）
            if (l1) l1 = l1->next;
            if (l2) l2 = l2->next;
        }

        return dummy.next; // 返回结果链表的真实头节点
    }
};
// @lc code=end

