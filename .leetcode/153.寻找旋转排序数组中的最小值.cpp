    int findMin(std::vector<int>& nums) {
        int left = 0, right = nums.size() - 1;
        
        while(left < right) {
            int mid = left + (right - left) / 2;
            if(nums[mid] > nums[right]) left = mid + 1;
            else if (nums[left] > nums[right]) ++left;
            else right = mid - 1;
        }
        return nums[left];
    }
/*
弄清楚这几个数据就能够写代码了
            1 2 3 4 5 6
            4 5 6 1 2 3
            7 1 2 3 4 5 6
            6 7 8 9 10 11 1 2
                    l  m    r
                          l r
                          m
            3 4 5 1 2
            l   m    r
                  l  r
                  m
            4 5 6 7 0 1 2
            l     m     r
                    l   r
                      m
            3 1 2
            l m r

            5 1 2 3 4
            l   m   r

            4,5,1,2,3

*/
class Solution {
public:
    int findMin(std::vector<int>& nums) {
        int size = nums.size();
        int left{ 0 }, right{ size - 1 };
        int lv = nums.front(), rv = nums.back();
        while (left <= right)
        {
            int mid = left + ((right - left) >> 1);
            int mid_v = nums[mid];

            if (mid_v > rv) left = mid + 1;
            else if (mid_v < lv) {
                // 分三种情况
                auto currlv = nums[left];
                // 如果mid是最小值（3,1,2），mid left差必须等于1（5 1 2 3 4）
                if (currlv > mid_v && (mid - left) == 1) return mid_v;
                // 如果当前left恰好是最小值，注意left大于0（4,5,1,2,3）
                if (left > 0 && nums[left - 1] > currlv) return currlv;
                // 否则移动
                ++left;
            }
            else return nums[left];
        }
        return -1; // 不会到达这里
    }
};
/*
 * @lc app=leetcode.cn id=153 lang=cpp
 *
 * [153] 寻找旋转排序数组中的最小值
 */

// @lc code=start
#include <climits>
#include <algorithm>
#include <vector>
using namespace std;
class Solution {
public:
    int findMin(vector<int>& nums) {
        int left = 0, right = nums.size() - 1;
        while (left <= right)
        {
            int mid = left + (right - left) / 2;
            if(nums[left] <= nums[mid]){
                if(nums[left] > nums[right]){
                    left = mid + 1;
                }
                else{
                    return nums[left];
                }
            }
            else{
                ++left;
                right = mid;
            }
        }
        return std::min(nums[left], nums[right]);
    }
};
// @lc code=end

