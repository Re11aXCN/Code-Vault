/*
 * @lc app=leetcode.cn id=82 lang=cpp
 *
 * [82] 删除排序链表中的重复元素 II
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
    ListNode* deleteDuplicates(ListNode* head) {
        if (!head || !head->next) return head;

        ListNode dummy(-101);
        dummy.next = head;
        ListNode* left = &dummy;
        ListNode* mid = head;
        ListNode* right = head->next;
        bool isCommon{ false };
        while (right) {
            if (mid->val == right->val) {
                isCommon = true;
                mid->next = right->next;
                delete right;
                right = mid->next;
            }
            else {
                if (isCommon) {
                    isCommon = false;
                    left->next = mid->next;
                    delete mid;
                    mid = left->next;
                    right = mid->next;
                }
                else {
                    left = left->next;
                    mid = mid->next;
                    right = right->next;
                }
            }
        }
        if (isCommon) {
            left->next = mid->next;
            delete mid;
            mid = left->next;
        }
        return dummy.next;
    }
};
// @lc code=end

int main()
{
    // [1, 2, 3, 3, 3]
    // [1,1,1,1]
    // [1,2,3,3,4,4,5]
    // [1,1,1,2,3]
    ListNode* head1 = new ListNode(1);
    head1->next = new ListNode(2);
    head1->next->next = new ListNode(3);
    head1->next->next->next = new ListNode(3);
    head1->next->next->next->next = new ListNode(3);

    ListNode* head2 = new ListNode(1);
    head2->next = new ListNode(1);
    head2->next->next = new ListNode(1);
    head2->next->next->next = new ListNode(1);

    ListNode* head3 = new ListNode(1);
    head3->next = new ListNode(2);
    head3->next->next = new ListNode(3);
    head3->next->next->next = new ListNode(3);
    head3->next->next->next->next = new ListNode(4);
    head3->next->next->next->next->next = new ListNode(4);
    head3->next->next->next->next->next->next = new ListNode(5);

    ListNode* head4 = new ListNode(1);
    head4->next = new ListNode(1);
    head4->next->next = new ListNode(1);
    head4->next->next->next = new ListNode(2);
    head4->next->next->next->next = new ListNode(3);

    Solution s;
    s.deleteDuplicates(head4);
    return 0;
}

