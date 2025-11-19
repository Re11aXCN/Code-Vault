/*
 * @lc app=leetcode.cn id=142 lang=cpp
 *
 * [142] 环形链表 II
 * 
 */
/*
n 3 2 0 4 5|2 0 4 5|2 0 4 5
    .
  s s s s s
  f   f   f   f   f

n 3 2 0 4 5 6|2 0 4 5 6|2 0 4 5 6
    .
  s s s s s s
  f   f   f   f   f   f

n 3 1 2 3 4 5 2 0 4 5 6|2 0 4 5 6|2 0 4 5 6
              .
  s s s s s s s s s s s
  f   f   f   f   f   f   f   f   f   f   f
  环长5，不是环 6 ，当前位置环的倒数第一个位置6

n 3 1 2 3 4 5 6 2 0 4 5 6|2 0 4 5 6|2 0 4 5 6
                .
  s s s s s s s s s s s
  f   f   f   f   f   f   f   f   f   f   f
  环长5，不是环 7 ，当前位置环的倒数第二个位置5

  
前 不是环的部分 长度 mod 环部分长度 得到余数，一定是环当前倒数 余数的位置
所以直接从头移动必定相遇
*/
// @lc code=start
/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */
class Solution {
public:
    ListNode *detectCycle(ListNode *head) {
        if(!head || !head->next) return nullptr;

        ListNode* slow = head;
        ListNode* fast = head;
        while(fast && fast->next) { 
            slow = slow->next, fast = fast->next->next;
            if(slow == fast) {
                fast = head;
                while(slow != fast) slow = slow->next, fast = fast->next;
                return fast;
            }
        }
        return nullptr;
    }
};
// @lc code=end

int main()
{
    ListNode* head = new ListNode(3);
    head->next = new ListNode(1);
    head->next->next = new ListNode(2);
    head->next->next->next = new ListNode(3);
    head->next->next->next->next = new ListNode(4);
    head->next->next->next->next->next = new ListNode(5);
    head->next->next->next->next->next->next = new ListNode(2);
    head->next->next->next->next->next->next->next = new ListNode(0);
    head->next->next->next->next->next->next->next->next = new ListNode(4);
    head->next->next->next->next->next->next->next->next->next = new ListNode(5);
    head->next->next->next->next->next->next->next->next->next->next = new ListNode(6);
    head->next->next->next->next->next->next->next->next->next->next->next = head->next->next->next->next->next->next;
    auto r = detectCycle(head);
    return 0;
}