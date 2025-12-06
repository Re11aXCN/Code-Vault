#include <vector>
#include <iostream>
using namespace std;

class Solution {
public:
    std::vector<int> searchRange(std::vector<int>& nums, int target) {
        // 初始化结果为[-1, -1]，处理未找到的情况
        std::vector<int> result = {-1, -1};
        if (nums.empty()) {
            return result;
        }

        // 第一步：找左边界（第一个等于target的位置）
        int left = findLeftBound(nums, target);
        // 若左边界不存在，直接返回[-1,-1]
        if (left == -1) {
            return result;
        }
        // 第二步：找右边界（最后一个等于target的位置）
        int right = findRightBound(nums, target);

        result[0] = left;
        result[1] = right;
        return result;
    }

private:
    // 二分查找左边界：第一个等于target的索引
    int findLeftBound(const vector<int>& nums, int target) {
        int left = 0;
        int right = nums.size() - 1;
        int leftBound = -1;

        while (left <= right) {
            int mid = left + (right - left) / 2; // 避免溢出
            if (nums[mid] == target) {
                leftBound = mid;       // 记录当前位置
                right = mid - 1;       // 继续向左找更小的索引
            } else if (nums[mid] < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return leftBound;
    }

    // 二分查找右边界：最后一个等于target的索引
    int findRightBound(const vector<int>& nums, int target) {
        int left = 0;
        int right = nums.size() - 1;
        int rightBound = -1;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (nums[mid] == target) {
                rightBound = mid;      // 记录当前位置
                left = mid + 1;        // 继续向右找更大的索引
            } else if (nums[mid] < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return rightBound;
    }
};

std::vector<int> searchRange(std::vector<int>& nums, int target) {
        int left = 0, right = nums.size() - 1;
        std::vector<int> result{-1, -1};
        while(left <= right) {
            int mid = left + (right - left) / 2;
            if (target > nums[mid]) left = mid + 1;
            else if (target < nums[mid]) right = mid - 1; 
            else {
                int t2 = mid, t1 = mid;
                helper2(nums, target, left, mid - 1, t2);
                helper1(nums, target, mid + 1, right, t1);
                result[0] = t2, result[1] = t1;
                break;
            }
        }
        return result;
    }
    void helper1(std::vector<int>& nums, int target, int left, int right, int& range) {
        while(left <= right) {
            int mid = left + (right - left) / 2;
            if (target > nums[mid]) left = mid + 1;
            else if (target < nums[mid]) right = mid - 1; 
            else {
                range = mid;
                helper1(nums, target, mid + 1, right, range);
                break;
            }
        }
    }
    void helper2(std::vector<int>& nums, int target, int left, int right, int& range) {
        while(left <= right) {
            int mid = left + (right - left) / 2;
            if (target > nums[mid]) left = mid + 1;
            else if (target < nums[mid]) right = mid - 1; 
            else {
                range = mid;
                helper2(nums, target, left, mid - 1, range);
                break;
            }
        }
    }

    std::vector<int> searchRange(std::vector<int>& nums, int target) {
        int size = nums.size();
        int left{0}, right{size - 1};
        while (left <= right)
        {
            int mid = left + ((right - left) >> 1);
            if(nums[mid] < target) left = mid + 1;
            else right = mid - 1;
        }
        // 注意我们大于/相等的时候是right移动，小于的时候是left移动
        // 所以找不到的情况为，left越界（先判断），即使right越界也没关系（因为相等），
        // 因为mid原因，判断left是否等于target即可
        // 5 8 8 8 8 8
        // 8 8 8 8 8 8
        if(left == size || nums[left] != target) return {-1,-1};
        std::vector<int> res(2, left);
        for(; left < size && nums[left] == target; ++left);
        res.back() = left - 1; // 注意for会多一次循环 满足 != 结束循环，所以减掉1
        return res;
    }
/*
 * @lc app=leetcode.cn id=34 lang=cpp
 *
 * [34] 在排序数组中查找元素的第一个和最后一个位置
 */

// @lc code=start
#include <vector>
#include <algorithm>
using namespace std;
class Solution {
public:
//我的思路是仅一次二分查找找目标，找到遍历左右得到边界
//最优解两次二分查找直接找左边界和右边界
    vector<int> searchRange(vector<int>& nums, int target) {
        auto left = lower_bound(nums.begin(), nums.end(), target);
        if (left == nums.end() || *left != target) return {-1, -1};
        auto right = upper_bound(nums.begin(), nums.end(), target);
        return {int(left - nums.begin()), int(right - nums.begin() - 1)};
    }
/*
    vector<int> searchRange(vector<int>& nums, int target) {
        // 处理空数组情况
        if (nums.empty()) return {-1, -1};
        
        // 查找左边界：第一个等于target的位置
        int left = findBound(nums, target, true);
        // 如果左边界未找到，直接返回[-1, -1]
        if (left == -1) return {-1, -1};
        
        // 查找右边界：最后一个等于target的位置
        int right = findBound(nums, target, false);
        return {left, right};
    }
*/
private:
    // 辅助函数：查找左或右边界
    // isLeft为true时找左边界，否则找右边界
    int findBound(const vector<int>& nums, int target, bool isLeft) {
        int left = 0, right = nums.size() - 1;
        int bound = -1;
        
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (nums[mid] == target) {
                bound = mid; // 记录当前位置
                if (isLeft) {
                    right = mid - 1; // 继续向左找更小的索引
                } else {
                    left = mid + 1;  // 继续向右找更大的索引
                }
            } else if (nums[mid] < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        return bound;
    }
};
// @lc code=end

