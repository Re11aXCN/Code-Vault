/*
 * @lc app=leetcode.cn id=33 lang=cpp
 *
 * [33] 搜索旋转排序数组
 */

// @lc code=start
class Solution {
public:
// 自己思路：循环找落差O(n)，两边二分法查找O(logn)
// 最优？直接二分查找，不断比较target移动指针，
    int search(vector<int>& nums, int target) {
        // ================= 解法选择开关 =================
//#define FIND_PIVOT_THEN_BINARY  // 方法一：先找旋转点再二分
#define DIRECT_BINARY_SEARCH   // 方法二：直接二分查找（最优解）

#ifdef FIND_PIVOT_THEN_BINARY
        /* 方法一：找旋转点后二分查找（时间复杂度O(2 logn)）*/
        int n = nums.size();
        if (n == 0) return -1;

        // 1. 查找旋转点（最小值位置）
        int left = 0, right = n - 1;
        while (left < right) {
            int mid = left + (right - left) / 2;
            if (nums[mid] > nums[right]) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
        int pivot = left;

        // 2. 确定目标值所在的区间
        left = 0;
        right = n - 1;
        if (target >= nums[pivot] && target <= nums[right]) {
            left = pivot; // 目标在右半段
        } else {
            right = pivot - 1; // 目标在左半段
        }

        // 3. 标准二分查找
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (nums[mid] == target) return mid;
            if (nums[mid] < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return -1;

#else
        /* 方法二：直接二分查找（时间复杂度O(logn)）*/
        int left = 0, right = nums.size() - 1;
        while (left <= right) {
            int mid = left + ((right - left) >> 1);
            
            if (nums[mid] == target) return mid;
            
            // 判断左半段是否有序
            if (nums[left] <= nums[mid]) { 
                // 目标值在左半有序区间内
                if (nums[left] <= target && target < nums[mid]) {
                    right = mid - 1;
                } else {
                    left = mid + 1;
                }
            } 
            // 否则右半段有序
            else { 
                // 目标值在右半有序区间内
                if (nums[mid] < target && target <= nums[right]) {
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            }
        }
        return -1;
#endif
    }
};
// @lc code=end

