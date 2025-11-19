/*
 * @lc app=leetcode.cn id=206 lang=cpp
 *
 * [206] 反转链表
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
// @lc code=start
#include <vector>
class Solution {
public:
/*
// 保留链表指针顺序，仅交换值
    ListNode* reverseList(ListNode* head) {
        if(!head || !head->next) return head;

        std::vector<int> vec;
        ListNode* ptr = head;
        while (ptr)
        {
            vec.emplace_back(ptr->val);
            ptr = ptr->next;
        }
        ptr = head;
        for(auto it = vec.rbegin(); it != vec.rend(); ++it){
            ptr->val = *it;
            ptr = ptr->next;
        }
        return head;
    }
*/
    // 1 2 3 4 5
    // 1 head
    // 2 curr 3 curr->next 2 prev 3 prev->next

    // 3 curr 4 curr->next 2 prev 3 prev->next
    // 3 curr 4 curr->next 2 prev 1 prev->next

    // head = prev    2 head
    // prev = curr;
    // 3 curr 4 curr->next 3 prev 4 prev->next

    // 4 curr 5 curr->next 3 prev 4 prev->next
    // 4 curr 5 curr->next 3 prev 2 prev->next

    // head = prev    3 head
    // prev = curr;
    // 4 curr 5 curr->next 4 prev 5 prev->next

    // 5 curr null curr->next 4 prev 5 prev->next
    // 5 curr null curr->next 4 prev 3 prev->next

    // head = prev    4 head
    // prev = curr;
    // 5 curr null curr->next 5 prev null prev->next


// 不保留链表指针顺序，交换指针指向
    ListNode* reverseList(ListNode* head) {
        // 处理0/1节点情况
        if(!head || !head->next) return head;

        // 处理2个节点情况
        ListNode* curr = head->next;
        ListNode* prev = curr;
        head->next = nullptr;
        if(!prev->next) {
            prev->next = head;
            return prev;
        }
        // 处理大于2个节点情况
        while (true)
        {
            curr = curr->next;
            prev->next = head;
            head = prev;
            prev = curr;
            if(!prev->next) {
                prev->next = head;
                break;
            }
        }
        
        return prev;
    }
// 优化代码
    ListNode* reverseList(ListNode* head) {
        ListNode* prev = nullptr;
        ListNode* next = nullptr;
        ListNode* curr = head;
        while (curr) {
            next = curr->next;
            curr->next = prev;
            prev = curr;
            curr = next;
        }
        return prev;
    }
};
// @lc code=end

/*
 * @lc app=leetcode.cn id=206 lang=cpp
 *
 * [206] 反转链表
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
 struct ListNode {
    int val;
    ListNode* next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode* next) : val(x), next(next) {}
    
};
#include <vector>
class Solution {
public:
    ListNode* reverseList(ListNode* head) {
#ifdef SPACE_ON
        if (!head) return nullptr;
        // 阶段1：遍历链表获取所有节点的值
        // ----------------------------------
        std::vector<int> values;
        ListNode* current = head;  // 使用临时指针遍历，保留原head位置
        while (current) {          // 正确遍历所有节点（原代码漏掉了最后一个节点）
            values.push_back(current->val);
            current = current->next;
        }

        // 阶段2：销毁原链表
        // -------------------------------
        current = head;  // 重置到链表头部
        while (current) {
            ListNode* temp = current;
            current = current->next;
            delete temp;  // 释放当前节点内存
        }

        // 阶段3：重建反转后的链表
        // -------------------------------
        ListNode* dummy = new ListNode(0);  // 使用哑节点简化操作
        current = dummy;

        // 逆序插入值（实现反转）
        for (auto it = values.rbegin(); it != values.rend(); ++it) {
            current->next = new ListNode(*it);
            current = current->next;
        }

        ListNode* newHead = dummy->next;
        delete dummy;  // 清理哑节点
        return newHead;
#else 
        ListNode* prev = nullptr;
        ListNode* current = head;

        while (current) {
            ListNode* nextNode = current->next;  // 保存下一节点
            current->next = prev;                // 反转指针
            prev = current;                      // 前移prev
            current = nextNode;                  // 前移current
        }
        return prev;  // 新链表头
#endif // SPACE_ON

    }
};
// @lc code=end