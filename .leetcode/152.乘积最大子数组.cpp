/*
 * @lc app=leetcode.cn id=152 lang=cpp
 *
 * [152] 乘积最大子数组
 */

// @lc code=start
#include <vector>
using namespace std;
class Solution {
public:
/*
- 当遇到负数时，最小乘积（可能是负数）乘以负数可能变成最大乘积
- 当遇到正数时，最大乘积乘以正数仍然是最大乘积
- 当遇到0时，重新开始计算（通过直接选择nums[i]实现）
*/
    int maxProduct(vector<int>& nums) {
        if (nums.empty()) return 0;
        
        // 初始化最大和最小乘积为第一个元素
        int max_so_far = nums[0];
        int min_so_far = nums[0];
        int result = nums[0];  // 全局最大乘积
        
        for (int i = 1; i < nums.size(); ++i) {
            // 保存前一步的最大值，因为在计算min_so_far时会用到
            int prev_max = max_so_far;
            
            // 更新最大乘积：可能是当前数、当前数乘以之前最大乘积、当前数乘以之前最小乘积
            max_so_far = max({nums[i], prev_max * nums[i], min_so_far * nums[i]});
            
            // 更新最小乘积
            min_so_far = min({nums[i], prev_max * nums[i], min_so_far * nums[i]});
            
            // 更新全局最大乘积
            result = max(result, max_so_far);
        }
        
        return result;
    }
};
// @lc code=end