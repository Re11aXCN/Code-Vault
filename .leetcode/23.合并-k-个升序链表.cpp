/*
    使用interval表示当前合并的步长

    每次合并间隔为interval的两个链表

    结果放在前一个位置（lists[i]）

    不断加倍interval直到覆盖所有链表
*/
class Solution {
public:
    ListNode* mergeKLists(vector<ListNode*>& lists) {
        if (lists.empty()) return nullptr;
    
        int k = lists.size();
        int interval = 1;  // 初始步长
    
        while (interval < k) {
            #pragma clang loop unroll_count(4)
            for (int i = 0; i < k - interval; i += interval * 2) {
                // 合并 lists[i] 和 lists[i + interval]
                lists[i] = merge(lists[i], lists[i + interval]);
            }
            interval *= 2;  // 步长加倍
        }
    
        return lists[0];
    }

    ListNode* merge(ListNode* l1, ListNode* l2) {
        ListNode dummy(0, nullptr), *tail = &dummy;
        while(l1 && l2) {
            if (l1->val <= l2->val) {
                tail->next = l1;
                l1 = l1->next;
            }
            else {
                tail->next = l2;
                l2 = l2->next;
            }
            tail = tail->next;
        }
        tail->next = l1 ? l1 : l2;
        return dummy.next;
    }
};
// or
ListNode* mergeKLists(vector<ListNode*>& lists) {
    if (lists.empty()) return nullptr;
    
    int n = lists.size();
    while (n > 1) {
        int i = 0;
        for (int j = 0; j < n; j += 2) {
            if (j + 1 < n) {
                // 合并相邻的两个链表，结果放在前一个位置
                lists[i] = merge(lists[j], lists[j + 1]);
            } else {
                // 如果是奇数个，直接复制最后一个
                lists[i] = lists[j];
            }
            i++;
        }
        n = i;  // 更新链表的数量（减半或减半加1）
    }
    
    return lists[0];
}
/*
 * @lc app=leetcode.cn id=23 lang=cpp
 *
 * [23] 合并 K 个升序链表
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
ListNode* mergeKLists(vector<ListNode*>& lists) {
        // ================= 解法选择开关 =================
#define DIVIDE_CONQUER  // 分治法 O(N logK) 时间，O(logK) 空间
//#define HEAP_METHOD    // 优先队列法 O(N logK) 时间，O(K) 空间

#ifdef DIVIDE_CONQUER
        /* 方法一：分治法（类似归并排序）*/
        return mergeHelper(lists, 0, lists.size() - 1);

#else
        /* 方法二：优先队列（最小堆）*/
        // 自定义比较函数，构建最小堆
        auto cmp = [](ListNode* a, ListNode* b) { return a->val > b->val; };
        priority_queue<ListNode*, vector<ListNode*>, decltype(cmp)> minHeap(cmp);

        // 初始化：将所有非空链表头加入堆
        for (ListNode* list : lists) {
            if (list) minHeap.push(list);
        }

        ListNode dummy(0); // 哑节点简化操作
        ListNode* current = &dummy;

        // 每次取出最小节点，并将其后继加入堆
        while (!minHeap.empty()) {
            ListNode* minNode = minHeap.top();
            minHeap.pop();
            current->next = minNode;
            current = current->next;
            if (minNode->next) {
                minHeap.push(minNode->next);
            }
        }

        return dummy.next;
#endif
    }

private:
#ifdef DIVIDE_CONQUER
    // 分治递归函数：合并lists[left..right]
    ListNode* mergeHelper(vector<ListNode*>& lists, int left, int right) {
        if (left > right) return nullptr;
        if (left == right) return lists[left];
        
        int mid = left + (right - left) / 2;
        ListNode* l1 = mergeHelper(lists, left, mid);
        ListNode* l2 = mergeHelper(lists, mid + 1, right);
        return mergeTwoLists(l1, l2);
    }

    // 合并两个有序链表（迭代版）
    ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
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
#endif
};
// @lc code=end

