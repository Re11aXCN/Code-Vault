ListNode* reverseKGroup(ListNode* head, int k) {
    if (head == nullptr || k == 1) return head;
    ListNode dummy(0, nullptr), *dummyHead = &dummy, *dummyTail = dummy.next, *first = head;
    int lLen = 0;
    while (first && first->next) {
        first = first->next->next;
        ++lLen;
    }
    if (lLen <<= 1; first != nullptr) ++lLen;
    int round = lLen / k;
    first = head;
    while (round-- != 0) {
        int count = k;
        while (count != 0) {
            if (count-- == k) dummyTail = first;
            ListNode* temp = first->next;
            first->next = dummyHead->next;
            dummyHead->next = first;
            first = temp;
        }
        dummyTail->next = first;
        dummyHead = dummyTail;
    }
    return dummy.next;
}
/*
 * @lc app=leetcode.cn id=25 lang=cpp
 *
 * [25] K 个一组翻转链表
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
    ListNode* reverseKGroup(ListNode* head, int k) {
        ListNode* curr = head;
        int size{ 0 };
        while (curr)
        {
            curr = curr->next;
            ++size;
        }
        if (k > size) return head;

        int round = (size / k) - 1;
        int count = k;

        ListNode* prev = nullptr;
        ListNode* next = nullptr;
        ListNode* ptr = nullptr;
        curr = head;
        while (count)
        {
            next = curr->next;
            curr->next = prev;
            prev = curr;
            curr = next;
            --count;
        }
        ListNode* res = prev;
        ptr = head;
        ptr->next = curr;
        prev = ptr;
        count = k;
        // 3 2 1 <=> 4 <- 5 <- 6  7 curr
        // 6 是 prev 1 是 ptr
        // 4 ptr->next 指向 7 curr
        // 1 ptr 指向 6 prev
        // 3 2 1 6 5 4 7 curr
        while (round)
        {
            while (count)
            {
                next = curr->next;
                curr->next = prev;
                prev = curr;
                curr = next;
                --count;
            }
            ListNode* temp = ptr->next;
            ptr->next->next = curr;
            ptr->next = prev;
            ptr = temp;
            --round;
            count = k;
        }

        return res;
    }
    
// 代码逻辑规范清晰
    ListNode* reverseKGroup(ListNode* head, int k) {
        // 计算链表长度
        int size = 0;
        ListNode* curr = head;
        while (curr) {
            curr = curr->next;
            ++size;
        }
        
        // 如果k大于链表长度，直接返回原链表
        if (k > size) {
            return head;
        }
        
        ListNode* dummy = new ListNode(0); // 创建哑节点简化边界处理
        dummy->next = head;
        ListNode* prev_group_tail = dummy; // 记录上一组的尾节点
        ListNode* curr_group_head = head;  // 当前组的头节点
        int rounds = size / k;             // 需要翻转的组数
        
        curr = curr_group_head;
        // 处理每一组翻转
        for (int i = 0; i < rounds; ++i) {
            ListNode* prev = nullptr;
            
            // 翻转当前组内的k个节点
            for (int j = 0; j < k; ++j) {
                ListNode* next = curr->next;
                curr->next = prev;
                prev = curr;
                curr = next;
            }
            
            // 连接上一组与当前翻转后的组
            prev_group_tail->next = prev;
            // 当前组翻转后的尾节点指向下一组的头
            curr_group_head->next = curr;
            
            // 更新指针位置为下一组准备
            prev_group_tail = curr_group_head;
            curr_group_head = curr;
        }
        
        ListNode* new_head = dummy->next;
        delete dummy;
        return new_head;
    }
};
 // @lc code=end
/*
 * @lc app=leetcode.cn id=25 lang=cpp
 *
 * [25] K 个一组翻转链表
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
    ListNode* reverseKGroup(ListNode* head, int k) {
//#define TWO_PASS_METHOD   // 两次遍历法
#define ONE_PASS_METHOD  // 单次遍历法

#ifdef TWO_PASS_METHOD
        /* 方法一：两次遍历法（统计长度+分组处理）*/
        if (!head || k == 1) return head;

        // 阶段1：计算链表总长度
        int length = 0;
        ListNode* curr = head;
        while (curr) {
            length++;
            curr = curr->next;
        }

        // 创建哑节点简化头节点处理
        ListNode dummy(0);
        dummy.next = head;
        ListNode* prev = &dummy;  // 当前组的前驱节点

        // 阶段2：按组翻转
        for (int groups = length / k; groups > 0; groups--) {
            // 翻转当前k个节点
            ListNode* group_head = prev->next;
            ListNode* group_tail = group_head;
            for (int i = 1; i < k; i++) 
                group_tail = group_tail->next;

            // 保存后续节点并断开当前组
            ListNode* next_group = group_tail->next;
            group_tail->next = nullptr;

            // 翻转当前组并重新连接
            prev->next = reverseList(group_head);
            group_head->next = next_group;  // 原头节点变为尾节点

            // 移动prev到下一组的前驱位置（当前组的尾）
            prev = group_head;
        }
        return dummy.next;

#else
        /* 方法二：单次遍历法（实时检测+条件翻转）*/
        ListNode dummy(0);
        dummy.next = head;
        ListNode* prev = &dummy;  // 当前组的前驱节点
        
        while (true) {
            // 检测剩余节点是否足够k个
            ListNode* check = prev;
            for (int i = 0; i < k; i++) {
                check = check->next;
                if (!check) return dummy.next; // 不足k个直接返回
            }

            // 翻转当前k个节点
            ListNode* group_head = prev->next;
            ListNode* curr = group_head;
            ListNode* next = nullptr;
            for (int i = 0; i < k; i++) {    // 标准头插法翻转
                ListNode* temp = curr->next;
                curr->next = next;
                next = curr;
                curr = temp;
            }

            // 重新连接链表
            prev->next = next;          // 前驱接新头
            group_head->next = curr;    // 新尾接后续节点
            prev = group_head;          // 更新前驱到新尾
        }
#endif
    }
private:
        // 辅助函数：翻转链表并返回新头节点
    ListNode* reverseList(ListNode* head) {
        ListNode* prev = nullptr;
        ListNode* curr = head;
        while (curr) {
            ListNode* next = curr->next;
            curr->next = prev;
            prev = curr;
            curr = next;
        }
        return prev;
    }
};
// @lc code=end

