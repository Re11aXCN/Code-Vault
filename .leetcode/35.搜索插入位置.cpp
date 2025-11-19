class Solution {
public:
    int searchInsert(std::vector<int>& nums, int target) {
        return lower_bound(nums.data(), nums.size(), target);
    }
private:
    int lower_bound(int* data, int length, int target)
    {
        int left{ 0 }, right{ length - 1 };

        while(left <= right) {
            int mid = left + ((right - left) >> 1);
            if(data[mid] > target) right = mid - 1;
            else if(data[mid] < target) left = mid + 1;
            else return mid;
        }
        return left;
    }
};
/*
 * @lc app=leetcode.cn id=35 lang=cpp
 *
 * [35] 搜索插入位置
 */

// @lc code=start
#include <algorithm>
using namespace std;
class Solution {
public:
    int searchInsert(vector<int>& nums, int target) {
        int left = 0;
        int right = nums.size() - 1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (nums[mid] < target) {
                left = mid + 1;  // 目标在右半部分
            } else {
                right = mid - 1; // 目标在左半部分或等于mid
            }
        }
        
        // 循环结束时，left即为插入位置
        return left;    
    }
};
// @lc code=end

