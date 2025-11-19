/*
 * @lc app=leetcode.cn id=53 lang=cpp
 *
 * [53] 最大子数组和
 */

// @lc code=start
#include <vector>
using namespace std;
class Solution {
public:
    // 只有负数，最大-1
    // 有负数，有0，最大是0
    // 有0，有正数，整体和为最大
    // 有负数，有0，有正数，最大要么是单个正数，要么就是多个正数+多个负数
    /*
    解决思路
        要找到最大子数组和，可以采用动态规划的思想，具体为Kadane算法。该算法通过维护当前最大子数组和和全局最大子数组和，在遍历数组时不断更新这两个值，从而在线性时间内解决问题。
    步骤解析：
        初始化：将当前最大和（current_max）和全局最大和（global_max）初始化为数组的第一个元素。
        遍历数组：从第二个元素开始，对于每个元素，决定是将其加入当前子数组还是以该元素作为新子数组的起点。
        更新最大值：每次更新当前最大和后，比较并更新全局最大和。
        返回结果：遍历结束后，全局最大和即为所求。
    */
    int maxSubArray(vector<int>& nums) {
        if (nums.empty()) return 0; // 处理空数组的情况

        int current_max = nums.front(); // 当前子数组的最大和，初始为第一个元素
        int global_max = current_max; // 全局最大和，初始也为第一个元素

        for(auto it = nums.begin() + 1; it != nums.end(); ++it) {
            // 决定是延续当前子数组还是以当前元素作为新起点
            current_max = std::max(*it, current_max + *it);
            // 更新全局最大值
            if(current_max > global_max) global_max = current_max;
        }

        return global_max;
    }

    // 前缀和
    int maxSubArray(vector<int>& nums) {
        int curr_pre = 0;      // 当前前缀和
        int min_pre = 0;       // 当前遇到的最小前缀和
        int max_sum = INT_MIN; // 全局最大子数组和
        #pragma clang loop unroll_count(4)
        for (int x : nums) {
            curr_pre += x;   // 更新前缀和
            if (int candidate = curr_pre - min_pre; candidate > max_sum) {
                max_sum = candidate; // 更新最大子数组和
            }
            if (curr_pre < min_pre) min_pre = curr_pre;   // 更新最小前缀和
        }
        return max_sum;
    }
};
// @lc code=end

