class Solution {
public:
    ListNode* sortList(ListNode* head) {
        // 递归终止条件：空节点或单个节点
        if (!head || !head->next) return head;
        
        // 使用快慢指针找到链表中点
        ListNode* slow = head;
        // 常规的fast是head，1 2 3 4偶数落在3（划分不均匀1 2 3 | 4），1 2 3 4 5奇数落在3（划分均匀1 2 3 | 4 5）
        // 所以得抢跑1，偶数（1 2 | 3 4），奇数（1 2 | 3 4 5）
        ListNode* fast = head->next; 
        
        while (fast && fast->next) {
            slow = slow->next;
            fast = fast->next->next;
        }
        
        // 分割链表
        fast = slow->next;
        slow->next = nullptr;
        slow = head;

        // 递归排序两个子链表
        slow = sortList(slow);
        fast = sortList(fast);
        
        // 合并两个有序链表
        return merge(slow, fast);
    }
    
private:
    // 合并两个有序链表
    ListNode* merge(ListNode* l1, ListNode* l2) {
        ListNode dummy(0);  // 哑节点，简化边界处理
        ListNode* tail = &dummy;
        
        while (l1 && l2) {
            if (l1->val <= l2->val) {
                tail->next = l1;
                l1 = l1->next;
            } else {
                tail->next = l2;
                l2 = l2->next;
            }
            tail = tail->next;
        }
        
        // 连接剩余部分
        tail->next = l1 ? l1 : l2;
        
        return dummy.next;
    }
};

class Solution {
public:
    ListNode* sortList(ListNode* head) {
        if (!head || !head->next) return head;

        int length{ 0 };
        ListNode* curr = head;
        while (curr) curr = curr->next, ++length;

        std::vector<ListNode*> v;
        v.reserve(length);

        curr = head;
        while (curr) v.push_back(curr), curr = curr->next;

        if (length >= 2048) {
            std::sort(std::execution::par_unseq, v.begin(), v.end(), [](auto& node1, auto& node2) { return node1->val < node2->val; });
        }
        else {
            std::sort(v.begin(), v.end(), [](auto& node1, auto& node2) { return node1->val < node2->val; });
        }
        v.back()->next = nullptr;
        #pragma GCC unroll 8
        for (auto it = v.begin(); it != v.end() - 1; ++it)
        {
            (*it)->next = *std::next(it);
        }
        return v.front();
    }
};
/*
 * @lc app=leetcode.cn id=148 lang=cpp
 *
 * [148] 排序链表
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
    ListNode* sortList(ListNode* head) {
       if (!head || !head->next) return head;

        // 计算链表长度
        int length = 0;
        ListNode* curr = head;
        while (curr) {
            length++;
            curr = curr->next;
        }

        ListNode dummy(0); // 哑节点简化操作
        dummy.next = head;
        
        // 自底向上归并，sub_len为当前子链表长度
        for (int sub_len = 1; sub_len < length; sub_len <<= 1) {
            ListNode* prev = &dummy; // 前一次合并后的尾节点
            ListNode* curr = dummy.next; // 当前处理节点
            
            // dummy(prev)  3->2->1->4
            // dummy  3  2  1->4
            // dummy->2->3(prev)
            // ...
            // dummy->2->3->1->4(prev), curr= nullptr
            // 
            // dummy(prev) 2->3 1-4
            // ...
            // dummy->1->2->3->4(prev), curr= nullptr
            // 
            while (curr) {
                // 分割出两个长度为sub_len的子链表
                ListNode* left = curr;
                ListNode* right = split(left, sub_len); // 第一个子链表的尾后节点
                curr = split(right, sub_len); // 第二个子链表的尾后节点
                
                // 合并两个子链表，并连接到前一次合并的尾部
                prev->next = merge(left, right);
                
                // 移动prev到当前合并链表的尾部
                while (prev->next) {
                    prev = prev->next;
                }
            }
        }
        return dummy.next;
    }

private:
    // 分割链表，返回后半部分的头节点，并断开前半部分的尾节点
    ListNode* split(ListNode* head, int n) {
        while (--n && head) {
            head = head->next;
        }
        if (!head) return nullptr;
        ListNode* next = head->next;
        head->next = nullptr; // 断开前半部分
        return next;
    }

    // 合并两个有序链表（迭代版）
    ListNode* merge(ListNode* l1, ListNode* l2) {
        ListNode dummy(0);
        ListNode* curr = &dummy;
        while (l1 && l2) {
            if (l1->val <= l2->val) {
                curr->next = l1;
                l1 = l1->next;
            } else {
                curr->next = l2;
                l2 = l2->next;
            }
            curr = curr->next;
        }
        curr->next = l1 ? l1 : l2;
        return dummy.next;
    }
};
// @lc code=end
int main() {
    Solution s;
    s.sortList(new ListNode(-1, new ListNode(5, new ListNode(3, new ListNode(4, new ListNode(0))))));
    return 0;
}
