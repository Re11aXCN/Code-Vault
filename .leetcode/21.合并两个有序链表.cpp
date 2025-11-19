class Solution {
public:
    ListNode* mergeTwoLists(ListNode* list1, ListNode* list2) {
        if (!list1 && !list2) return nullptr;
        if (!list1) return list2;
        if (!list2) return list1;

        ListNode dummy(0, nullptr);

        // 头插法
        ListNode* head = &dummy, *p1 = list1, *p2 = list2, *prev = head;
        head->next = list1;
        // dummy->5
        // 1 2 4
        // 1->5 dummy->1 dummy->1->5 ... dummy->1->2->5 dummy->1->2->->5
        while(p1 && p2) {
            // 始终是 链表2的插入链表1的位置
            if (p1->val > p2->val) {
                ListNode* temp = p2;
                p2 = p2->next;
                temp->next = p1;
                prev->next = temp;
                prev = temp;
            }
            else { // 小于等于我们就移动 p1让 p1->val > p2->val 成立
                prev = p1;
                p1 = p1->next;
            }
        }
        // 最后一定是p1是空，最加p2即可
        if(p2) prev->next = p2;
        return head->next;
    }
};
class Solution {
public:
    ListNode* mergeTwoLists(ListNode* list1, ListNode* list2) {
        if (!list1 && !list2) return nullptr;
        else if (!list1 && list2) return list2;
        else if (list1 && !list2) return list1;
        else {
            ListNode* dummy = new ListNode(0);
            ListNode* ptr = dummy;
            while (list1 && list2) {
                if (list1->val <= list2->val) {
                    ptr->next = list1;
                    ptr = ptr->next;
                    list1 = list1->next;
                }
                else {
                    ptr->next = list2;
                    ptr = ptr->next;
                    list2 = list2->next;
                }
            }
            if(list1 && !list2) ptr->next = list1;
            if(!list1 && list2) ptr->next = list2;
            ListNode* head = dummy->next;
            delete dummy;
            return head;
        }
    }
};
// @lc code=end
/*
 * @lc app=leetcode.cn id=21 lang=cpp
 *
 * [21] 合并两个有序链表
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
    ListNode* mergeTwoLists(ListNode* list1, ListNode* list2) {
         // 处理空链表的情况
        if (!list1) return list2;
        if (!list2) return list1;

        // 创建哑节点作为合并后链表的头前驱
        ListNode dummy(0);
        ListNode* current = &dummy; // 当前指针用于构建新链表

        // 双指针遍历两个链表
        while (list1 && list2) {
            if (list1->val <= list2->val) {
                // 连接list1节点并移动指针
                current->next = list1;
                list1 = list1->next;
            } else {
                // 连接list2节点并移动指针
                current->next = list2;
                list2 = list2->next;
            }
            current = current->next; // 移动当前指针
        }

        // 处理剩余节点（直接链接未遍历完的链表）
        current->next = list1 ? list1 : list2;

        return dummy.next; // 返回合并后的头节点
    }
};
// @lc code=end

