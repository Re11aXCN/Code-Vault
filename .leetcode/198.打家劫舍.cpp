/*
 * @lc app=leetcode.cn id=198 lang=cpp
 *
 * [198] 打家劫舍
 */

// @lc code=start
#include <vector>
#include <algorithm>
using namespace std;

class Solution {
public:
    // 可以间隔1个或者1个以上进行偷窃，只考虑奇偶不对2 1 1 2是4
    /*
- 问题本质 ：打家劫舍问题是一个经典的动态规划问题，核心约束是不能抢劫相邻的房子。
- 状态定义 ：
- 我们可以定义 dp[i] 表示考虑前i个房子能获得的最大金额。
- 对于每个房子，有两种选择：抢或不抢。

- 状态转移方程 ：
- 如果抢第i个房子，那么不能抢第i-1个房子，最大金额为 dp[i-2] + nums[i]
- 如果不抢第i个房子，那么最大金额为 dp[i-1]
- 因此， dp[i] = max(dp[i-2] + nums[i], dp[i-1])
    */
    int rob(vector<int>& nums) {
        int n = nums.size();
        
        // 处理边界情况
        if (n == 0) return 0;
        if (n == 1) return nums[0];
        
        // 使用两个变量代替dp数组，降低空间复杂度
        // prev2表示dp[i-2]，prev1表示dp[i-1]，curr表示dp[i]
        int prev2 = 0;       // 初始化为0，表示没有房子时的最大金额
        int prev1 = nums[0];  // 只有一个房子时的最大金额
        int curr = 0;
        
        // 动态规划过程
        for (int i = 1; i < n; ++i) {
            // 当前位置的最大金额 = max(抢当前房子+前前房子的最大金额, 不抢当前房子的最大金额)
            curr = max(prev2 + nums[i], prev1);
            
            // 更新状态，为下一次迭代做准备
            prev2 = prev1;
            prev1 = curr;
        }
        
        return prev1; // 返回最后一个状态，即所有房子考虑完后的最大金额
    }
};
// @lc code=end