    ListNode *getIntersectionNode(ListNode *headA, ListNode *headB) {
        auto getListSize = [](ListNode* head) static -> size_t {
            ListNode *fast = head, *slow = head;
            int size{ 0 };
            while(fast && fast->next) {
                slow = slow->next;
                fast = fast->next->next;
                ++size;
            }
            return fast ? (size << 1) + 1 : size << 1;
        };

        size_t sizeA = getListSize(headA), sizeB = getListSize(headB);
        ListNode *smallerSizeList = headA, *largerSizeList = headB;
        if (sizeA > sizeB) {
            std::swap(sizeA, sizeB);
            std::swap(smallerSizeList, largerSizeList);
        }
        while(sizeB-- != sizeA) largerSizeList = largerSizeList->next;

        while(smallerSizeList && smallerSizeList != largerSizeList) {
            smallerSizeList = smallerSizeList->next;
            largerSizeList = largerSizeList->next;
        }
        return smallerSizeList == nullptr ? nullptr : smallerSizeList;
    }
/*
 * @lc app=leetcode.cn id=160 lang=cpp
 *
 * [160] 相交链表
 */

// @lc code=start
class Solution {
public:
    ListNode *getIntersectionNode(ListNode *headA, ListNode *headB) {
        if(headA && headB) {
            ListNode* ptrA = headA;
            ListNode* ptrB = headB;
            int maxALen = 1, maxBLen = 1;
            while(ptrA || ptrB) {
                if(ptrA){
                    ptrA = ptrA->next;
                    ++maxALen;
                }
                if(ptrB){
                    ptrB = ptrB->next;
                    ++maxBLen;
                } 
            }
            // 确保 A 最长
            if(maxALen < maxBLen) std::swap(headA, headB);

            int diff = std::abs(maxALen - maxBLen);
            ptrA = headA;
            ptrB = headB;
            while(diff) { ptrA = ptrA->next; --diff; }

            while(ptrA != ptrB) {
                ptrA = ptrA->next;
                ptrB = ptrB->next;
            }

            // 还原
            if(maxALen < maxBLen) std::swap(headA, headB);

            return ptrA ? ptrA : nullptr;
        }
        else if(!headA && headB) return headB;
        else if(headA && !headB) return headA;
        else return nullptr;
    }
};
#include <vector>
/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */

//0 1 2 3 4
//7 3 4
class Solution {
public:
    ListNode *getIntersectionNode(ListNode *headA, ListNode *headB) {
        if (!headA || !headB) return nullptr;

        // 步骤1：计算两个链表的长度
        int lenA = getListLength(headA);
        int lenB = getListLength(headB);

        // 步骤2：对齐起点，让长链表先走差值步
        ListNode *pa = headA, *pb = headB;
        if (lenA > lenB) {
            for (int i = 0; i < lenA - lenB; ++i) pa = pa->next;
        } else {
            for (int i = 0; i < lenB - lenA; ++i) pb = pb->next;
        }

        // 步骤3：同步遍历比较节点
        while (pa && pb) {
            if (pa == pb) return pa; // 找到交点
            pa = pa->next;
            pb = pb->next;
        }

        return nullptr; // 没有交点
    }

private:
    // 辅助函数：计算链表长度
    int getListLength(ListNode* head) {
        int length = 0;
        while (head) {
            ++length;
            head = head->next;
        }
        return length;
    }
/*
    ListNode *getIntersectionNode(ListNode *headA, ListNode *headB) {
        if(!headA || !headB) return nullptr;

        ListNode* A = headA;
        ListNode* B = headB;
        if(A == B) return A;
        int ALength = 0, BLength = 0;
        while(A) {
            A = A->next;
            ++ALength;
        }
        while(B) {
            B = B->next;
            ++BLength;
        }
        A = headA;
        B = headB;
        if(ALength < BLength) {
            while (ALength != BLength)
            {
                B = B->next;
                --BLength;
            }
        }
        else if(ALength > BLength) {
            while (ALength != BLength)
            {
                A = A->next;
                --ALength;
            }
        }
        if (A == B && A != nullptr) return A;
        while(B != A){
            A = A->next;
            B = B->next;
            if(A == B && A != nullptr) return A;
        }
        return nullptr;
    }
        */
};
// @lc code=end
int main() {
    std::vector<int> a{ 4,1,8,4,5 };
    std::vector<int> b{ 5,6,1,8,4,5 };
    /*
    ListNode* headA = new ListNode(4);
    ListNode* headB = new ListNode(5);
    headA->next = new ListNode(1);
    headA->next->next = new ListNode(8);
    headA->next->next->next = nullptr;

    headB->next = new ListNode(6);
    headB->next->next = new ListNode(1);
    headB->next->next->next = headA->next->next;
    headB->next->next->next->next = nullptr;
    */
    
    ListNode* headB = new ListNode(2);
    headB->next = new ListNode(3);
    ListNode* headA = headB->next;
    Solution s;
    s.getIntersectionNode(headA, headB);
}
