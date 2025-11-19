/*
 * @lc app=leetcode.cn id=141 lang=cpp
 *
 * [141] 环形链表
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
//时间O(n)，空间O(n)，使用unordered_set存地址，判断是否有重复地址
//时间O(n(n+1)/2)，空间O(1)，每个节点都循环一次
//时间O(n)，空间O(1)，快慢指针法
// 数学原理
// 无环情况：快指针先到达链表尾部，时间复杂度O(n)
// 有环情况：
//      设环前长度 L，环长 C，快慢指针将在 O(L + C) 时间内相遇
//      因快指针相对慢指针每次接近1步，最多绕环1周即可相遇
    bool hasCycle(ListNode *head) {
        if(!head || !head->next) return false;
        if (head == head->next) return true;
        // ========== 解法选择开关 ==========
#define FAST_SLOW_POINTER  // 最优解：快慢指针法
//#define HASH_METHOD       // 哈希表法
//#define BRUTE_FORCE       // 暴力解法

#ifdef HASH_METHOD
        /* 方法一：哈希表法 时间复杂度O(n) 空间复杂度O(n) */
        unordered_set<ListNode*> visited;
        while (head) {
            if (visited.count(head)) return true; // 发现重复节点→有环
            visited.insert(head);
            head = head->next;
        }
        return false;

#elif defined(BRUTE_FORCE)
        /* 方法二：暴力解法 时间复杂度O(n²) 空间复杂度O(1) */
        ListNode* outer = head;
        int steps = 0; // 已检查的节点数

        while (outer) {
            ListNode* inner = head;
            // 只检查前steps个节点是否与当前outer节点相同
            for (int i = 0; i < steps; ++i) {
                if (inner == outer) return true; // 发现环
                inner = inner->next;
            }
            outer = outer->next;
            ++steps;
        }
        return false;

#else
        /* 方法三：快慢指针法（最优解） 时间复杂度O(n) 空间复杂度O(1) */

        ListNode* slow = head;       // 慢指针：每次移动1步
        ListNode* fast = head->next; // 快指针：每次移动2步

        while (fast && fast->next) { // fast更快到达末端，fast->next防止不是环的时候值是nullptr，使用nullptr->next是未定义行为
            if (slow == fast) return true; // 两指针相遇→有环
            slow = slow->next;
            fast = fast->next->next;
        }
        return false; // 快指针走到链表末端→无环
#endif
    }
};
// @lc code=end
int main() {
    Solution s;
    //-1,-7,7,-4,19,6,-9,-5,-2,-5
    ListNode* head = new ListNode(-1);
    head->next = new ListNode(-7);
    head->next->next = new ListNode(7);
    head->next->next->next = new ListNode(-4);
    head->next->next->next->next = new ListNode(19);
    head->next->next->next->next->next = new ListNode(6);
    head->next->next->next->next->next->next = new ListNode(-9);
    head->next->next->next->next->next->next->next = new ListNode(-5);
    head->next->next->next->next->next->next->next->next = new ListNode(-2);
    head->next->next->next->next->next->next->next->next->next = new ListNode(-5);
    head->next->next->next->next->next->next->next->next->next->next = head->next->next->next->next->next->next;
    //ListNode* head = new ListNode(1);
    //head->next = new ListNode(2);
    //head->next = new ListNode(2);
    //head->next->next = head;
    //head->next->next = new ListNode(0);
    //head->next->next->next = new ListNode(-4);
    //head->next->next->next->next = head->next->next;
    bool res = s.hasCycle(head);
    return 0;
}


