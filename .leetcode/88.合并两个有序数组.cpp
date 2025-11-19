/*
 * @lc app=leetcode.cn id=88 lang=cpp
 *
 * [88] 合并两个有序数组
 */

// @lc code=start
#include <vector>
#include <algorithm>
using std::vector;
class Solution {
public:
    void merge(vector<int>& nums1, int m, vector<int>& nums2, int n) {
        if (n == 0) return;
        if (m == 0) {
            std::swap(nums1, nums2);
            //std::copy(nums2.begin(), nums2.end(), nums1.begin());
            return;
        }
        auto it = nums1.rbegin(), it1 = nums1.rbegin() + n, it2 = nums2.rbegin();
        for ( ; it2 != nums2.rend() && it1 != nums1.rend(); ++it)
        {
            if (*it1 < *it2) {
                std::iter_swap(it2, it);
                ++it2;
            }
            else if (*it1 > *it2) {
                std::iter_swap(it1, it);
                ++it1;
            }
            else {
                std::iter_swap(it2, it);
                ++it;
                std::iter_swap(it1, it);
                ++it1, ++it2;
            }
        }
        if (it2 != nums2.rend()) {
            std::copy(it2, nums2.rend(), it);
        }
    }
};
// @lc code=end

// 第二次写这道题目的思路，比上述的慢一些，慢1.5 到2倍
void merge(vector<int>& nums1, int m, vector<int>& nums2, int n) {
    if(n == 0) return;
    int size1 = nums1.size(), size2 = nums2.size();
    int pos1 = size1 - size2 - 1;
    if(m == 0 || nums2[0] >= nums1[pos1]) {
        std::copy(nums2.begin(), nums2.end(), nums1.begin() + pos1 + 1);
        return;
    }
    if (nums2.back() <= nums1.front()) {
        std::copy(nums1.begin(), nums1.begin() + pos1 + 1, nums1.begin() + size2);
        std::copy(nums2.begin(), nums2.end(), nums1.begin());
        return;
    }
    int back = size1 - 1, pos2 = size2 - 1;
    for( ; pos2 >= 0 && pos1 >= 0; ) {
        nums1[back--] = nums1[pos1] >= nums2[pos2] ? nums1[pos1--] : nums2[pos2--];
    }
    if(pos1 < 0) {
        std::copy(nums2.begin(), nums2.begin() + pos2 + 1, nums1.begin());
    }
    /*
    auto it1 = nums1.rbegin() + n;
    auto it2 = nums2.rbegin();
    auto dest = nums1.rbegin();
    
    while (it1 != nums1.rend() && it2 != nums2.rend()) {
        *dest++ = *it1 > *it2 ? *it1++ : *it2++;
    }
    
    // 处理剩余元素
    while (it2 != nums2.rend()) {
        *dest++ = *it2++;
    }
    */
}

